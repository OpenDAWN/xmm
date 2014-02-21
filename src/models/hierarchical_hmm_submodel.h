//
//  hierarchical_hmm_submodel.h
//  mhmm
//
//  Created by Jules Francoise on 08/08/13.
//
//

#ifndef mhmm_hierarchical_hmm_submodel_h
#define mhmm_hierarchical_hmm_submodel_h

#include "hmm.h"

#define HHMM_DEFAULT_EXITTRANSITION 0.1
#define HHMM_DEFAULT_EXITPROBABILITY 0.1

using namespace std;

struct HHMMResults : public HMMResults {
    double exitLikelihood;
    double likelihoodnorm;
};

template<bool ownData> class HierarchicalHMM;

template<bool ownData>
class HierarchicalHMMSubmodel : public HMM<ownData>
{
    friend class HierarchicalHMM<ownData>;
public:
    HHMMResults results_h;
    
    // Forward variables
	vector<double> alpha_h[3] ; //!< Alpha variable (forward algorithm)
    
#pragma mark -
#pragma mark Constructors
    HierarchicalHMMSubmodel(TrainingSet< Phrase<ownData, 1> > *_trainingSet=NULL,
                             int nbStates_ = HMM_DEFAULT_NB_STATES,
                             int nbMixtureComponents_ = GMM_DEFAULT_NB_MIXTURE_COMPONENTS,
                             float covarianceOffset_ = GMM_DEFAULT_COVARIANCE_OFFSET)
    : HMM<ownData>(_trainingSet, nbStates_, nbMixtureComponents_, covarianceOffset_)
    {
        updateExitProbabilities(NULL);
    }
    
    virtual ~HierarchicalHMMSubmodel()
    {
        for (int i=0 ; i<3 ; i++)
            this->alpha_h[i].clear();
        exitProbabilities.clear();
    }
    
#pragma mark -
#pragma mark Exit Probabilities
    void updateExitProbabilities(float *_exitProbabilities = NULL)
    {
        if (_exitProbabilities == NULL) {
            exitProbabilities.resize(this->nbStates, 0.0);
            exitProbabilities[this->nbStates-1] = HHMM_DEFAULT_EXITPROBABILITY;
        } else {
            exitProbabilities.resize(this->nbStates, 0.0);
            for (int i=0 ; i < this->nbStates ; i++)
                try {
                    exitProbabilities[i] = _exitProbabilities[i];
                } catch (exception &e) {
                    throw RTMLException("Error reading exit probabilities", __FILE__, __FUNCTION__, __LINE__);
                }
        }
    }
    
    void addExitPoint(int state, float proba)
    {
        if (state >= this->nbStates)
            throw RTMLException("State index out of bounds", __FILE__, __FUNCTION__, __LINE__);
        exitProbabilities[state] = proba;
    }
    
#pragma mark -
#pragma mark Play !
    void initPlaying()
    {
        HMM<ownData>::initPlaying();
        for (int i=0 ; i<3 ; i++)
            alpha_h[i].resize(this->nbStates, 0.0);
    }
    
#pragma mark -
#pragma mark JSON I/O
    /*!
     Write to JSON Node
     */
    virtual JSONNode to_json() const
    {
        JSONNode json_hhmmsub(JSON_NODE);
        json_hhmmsub.set_name("HHMM SubModel");
        
        // Write Parent: EM Learning Model
        JSONNode json_hmm = HMM<ownData>::to_json();
        json_hmm.set_name("parent");
        json_hhmmsub.push_back(json_hmm);

        // Exit probabilities
        json_hhmmsub.push_back(vector2json(exitProbabilities, "exitProbabilities"));
        
        return json_hhmmsub;
    }
    
    /*!
     Read from JSON Node
     */
    virtual void from_json(JSONNode root)
    {
        try {
            assert(root.type() == JSON_NODE);
            JSONNode::iterator root_it = root.begin();
            
            // Get Parent: Concurrent models
            assert(root_it != root.end());
            assert(root_it->name() == "parent");
            assert(root_it->type() == JSON_NODE);
            HMM<ownData>::from_json(*root_it);
            root_it++;
            
            // Get Exit probabilities
            assert(root_it != root.end());
            assert(root_it->name() == "exitProbabilities");
            assert(root_it->type() == JSON_ARRAY);
            json2vector(*root_it, exitProbabilities, this->nbStates);
            
        } catch (exception &e) {
            throw RTMLException("Error reading JSON, Node: " + root.name() + " >> " + e.what());
        }
        
        this->trained = true;
    }

    
#pragma mark -
#pragma mark protected Attributes
protected:
    vector<float> exitProbabilities;
};


#endif