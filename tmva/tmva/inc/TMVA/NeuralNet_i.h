#ifndef TMVA_NEURAL_NET_I
#define TMVA_NEURAL_NET_I
#pragma once


#include <tuple>
#include <future>


namespace TMVA
{
    namespace DNN
    {








        template <typename T>
            T uniformFromTo (T from, T to)
        {
            return from + (rand ()* (to - from)/RAND_MAX);
        }



        template <typename Container, typename T>
            void uniformDouble (Container& container, T maxValue)
        {
            for (auto it = begin (container), itEnd = end (container); it != itEnd; ++it)
            {
//        (*it) = uniformFromTo (-1.0*maxValue, 1.0*maxValue);
                (*it) = TMVA::DNN::uniformFromTo (-1.0*maxValue, 1.0*maxValue);
            }
        }


        static std::shared_ptr<std::function<double(double)>> ZeroFnc = std::make_shared<std::function<double(double)>> ([](double /*value*/){ return 0; });


        static std::shared_ptr<std::function<double(double)>> Sigmoid = std::make_shared<std::function<double(double)>> ([](double value){ value = std::max (-100.0, std::min (100.0,value)); return 1.0/(1.0 + std::exp (-value)); });
        static std::shared_ptr<std::function<double(double)>> InvSigmoid = std::make_shared<std::function<double(double)>> ([](double value){ double s = (*Sigmoid.get ()) (value); return s*(1.0-s); });

        static std::shared_ptr<std::function<double(double)>> Tanh = std::make_shared<std::function<double(double)>> ([](double value){ return tanh (value); });
        static std::shared_ptr<std::function<double(double)>> InvTanh = std::make_shared<std::function<double(double)>> ([](double value){ return 1.0 - std::pow (value, 2.0); });

        static std::shared_ptr<std::function<double(double)>> Linear = std::make_shared<std::function<double(double)>> ([](double value){ return value; });
        static std::shared_ptr<std::function<double(double)>> InvLinear = std::make_shared<std::function<double(double)>> ([](double /*value*/){ return 1.0; });

        static std::shared_ptr<std::function<double(double)>> SymmReLU = std::make_shared<std::function<double(double)>> ([](double value){ const double margin = 0.3; return value > margin ? value-margin : value < -margin ? value+margin : 0; });
        static std::shared_ptr<std::function<double(double)>> InvSymmReLU = std::make_shared<std::function<double(double)>> ([](double value){ const double margin = 0.3; return value > margin ? 1.0 : value < -margin ? 1.0 : 0; });

        static std::shared_ptr<std::function<double(double)>> ReLU = std::make_shared<std::function<double(double)>> ([](double value){ const double margin = 0.0; return value > margin ? value-margin : 0; });
        static std::shared_ptr<std::function<double(double)>> InvReLU = std::make_shared<std::function<double(double)>> ([](double value){ const double margin = 0.0; return value > margin ? 1.0 : 0; });

        static std::shared_ptr<std::function<double(double)>> SoftPlus = std::make_shared<std::function<double(double)>> ([](double value){ return std::log (1.0+ std::exp (value)); });
        static std::shared_ptr<std::function<double(double)>> InvSoftPlus = std::make_shared<std::function<double(double)>> ([](double value){ return 1.0 / (1.0 + std::exp (-value)); });

        static std::shared_ptr<std::function<double(double)>> TanhShift = std::make_shared<std::function<double(double)>> ([](double value){ return tanh (value-0.3); });
        static std::shared_ptr<std::function<double(double)>> InvTanhShift = std::make_shared<std::function<double(double)>> ([](double value){ return 0.3 + (1.0 - std::pow (value, 2.0)); });

        static std::shared_ptr<std::function<double(double)>> SoftSign = std::make_shared<std::function<double(double)>> ([](double value){ return value / (1.0 + fabs (value)); });
        static std::shared_ptr<std::function<double(double)>> InvSoftSign = std::make_shared<std::function<double(double)>> ([](double value){ return std::pow ((1.0 - fabs (value)),2.0); });

        static std::shared_ptr<std::function<double(double)>> Gauss = std::make_shared<std::function<double(double)>> ([](double value){ const double s = 6.0; return exp (-std::pow(value*s,2.0)); });
        static std::shared_ptr<std::function<double(double)>> InvGauss = std::make_shared<std::function<double(double)>> ([](double value){ const double s = 6.0; return -2.0 * value * s*s * (*Gauss.get ()) (value); });

        static std::shared_ptr<std::function<double(double)>> GaussComplement = std::make_shared<std::function<double(double)>> ([](double value){ const double s = 6.0; return 1.0 - exp (-std::pow(value*s,2.0)); });
        static std::shared_ptr<std::function<double(double)>> InvGaussComplement = std::make_shared<std::function<double(double)>> ([](double value){ const double s = 6.0; return +2.0 * value * s*s * (*GaussComplement.get ()) (value); });



/*! \brief apply weights using drop-out
 *
 * itDrop correlates with itSourceBegin 
 */
        template <typename ItSource, typename ItWeight, typename ItTarget, typename ItDrop>
            void applyWeights (ItSource itSourceBegin, ItSource itSourceEnd,
                               ItWeight itWeight,
                               ItTarget itTargetBegin, ItTarget itTargetEnd,
                               ItDrop itDrop)
        {
            for (auto itSource = itSourceBegin; itSource != itSourceEnd; ++itSource)
            {
                for (auto itTarget = itTargetBegin; itTarget != itTargetEnd; ++itTarget)
                {
                    if (*itDrop)
                        (*itTarget) += (*itSource) * (*itWeight);
                    ++itWeight;
                }
                ++itDrop;        
            }
        }



/*! \brief apply weights without drop-out
 *
 * 
 */
        template <typename ItSource, typename ItWeight, typename ItTarget>
            void applyWeights (ItSource itSourceBegin, ItSource itSourceEnd,
                               ItWeight itWeight,
                               ItTarget itTargetBegin, ItTarget itTargetEnd)
        {
            for (auto itSource = itSourceBegin; itSource != itSourceEnd; ++itSource)
            {
                for (auto itTarget = itTargetBegin; itTarget != itTargetEnd; ++itTarget)
                {
                    (*itTarget) += (*itSource) * (*itWeight);
                    ++itWeight;
                }
            }
        }




/*! \brief apply weights backwards (for backprop)
 *
 * 
 */
        template <typename ItSource, typename ItWeight, typename ItPrev>
            void applyWeightsBackwards (ItSource itCurrBegin, ItSource itCurrEnd,
                                        ItWeight itWeight,
                                        ItPrev itPrevBegin, ItPrev itPrevEnd)
        {
            for (auto itPrev = itPrevBegin; itPrev != itPrevEnd; ++itPrev)
            {
                for (auto itCurr = itCurrBegin; itCurr != itCurrEnd; ++itCurr)
                {
                    (*itPrev) += (*itCurr) * (*itWeight);
                    ++itWeight;
                }
            }
        }



/*! \brief apply weights backwards (for backprop)
 *
 * itDrop correlates with itPrev (to be in agreement with "applyWeights" where it correlates with itSources (same node as itTarget here in applyBackwards)
 */
        template <typename ItSource, typename ItWeight, typename ItPrev, typename ItDrop>
            void applyWeightsBackwards (ItSource itCurrBegin, ItSource itCurrEnd,
                                        ItWeight itWeight,
                                        ItPrev itPrevBegin, ItPrev itPrevEnd,
                                        ItDrop itDrop)
        {
            for (auto itPrev = itPrevBegin; itPrev != itPrevEnd; ++itPrev)
            {
                for (auto itCurr = itCurrBegin; itCurr != itCurrEnd; ++itCurr)
                {
                    if (*itDrop)
                        (*itPrev) += (*itCurr) * (*itWeight);
                    ++itWeight; 
                }
                ++itDrop;
            }
        }





/*! \brief apply the activation functions 
 *
 * 
 */

        template <typename ItValue, typename Fnc>
            void applyFunctions (ItValue itValue, ItValue itValueEnd, Fnc fnc)
        {
            while (itValue != itValueEnd)
            {
                auto& value = (*itValue);
                value = (*fnc.get ()) (value);

                ++itValue; 
            }
        }


/*! \brief apply the activation functions and compute the gradient
 *
 * 
 */
        template <typename ItValue, typename Fnc, typename InvFnc, typename ItGradient>
            void applyFunctions (ItValue itValue, ItValue itValueEnd, Fnc fnc, InvFnc invFnc, ItGradient itGradient)
        {
            while (itValue != itValueEnd)
            {
                auto& value = (*itValue);
                value = (*fnc.get ()) (value);
                (*itGradient) = (*invFnc.get ()) (value);
        
                ++itValue; ++itGradient;
            }
        }



/*! \brief update the gradients
 *
 * 
 */
        template <typename ItSource, typename ItDelta, typename ItTargetGradient, typename ItGradient>
            void update (ItSource itSource, ItSource itSourceEnd, 
                         ItDelta itTargetDeltaBegin, ItDelta itTargetDeltaEnd, 
                         ItTargetGradient itTargetGradientBegin, 
                         ItGradient itGradient)
        {
            while (itSource != itSourceEnd)
            {
                auto itTargetDelta = itTargetDeltaBegin;
                auto itTargetGradient = itTargetGradientBegin;
                while (itTargetDelta != itTargetDeltaEnd)
                {
                    (*itGradient) += - (*itTargetDelta) * (*itSource) * (*itTargetGradient);
                    ++itTargetDelta; ++itTargetGradient; ++itGradient;
                }
                ++itSource; 
            }
        }




/*! \brief compute the regularization (L1, L2)
 *
 * 
 */
        template <EnumRegularization Regularization>
            inline double computeRegularization (double weight, const double& factorWeightDecay)
        {
            return 0;
        }

// L1 regularization
        template <>
            inline double computeRegularization<EnumRegularization::L1> (double weight, const double& factorWeightDecay)
        {
            return weight == 0.0 ? 0.0 : std::copysign (factorWeightDecay, weight);
        }

// L2 regularization
        template <>
            inline double computeRegularization<EnumRegularization::L2> (double weight, const double& factorWeightDecay)
        {
            return factorWeightDecay * weight;
        }


/*! \brief update the gradients, using regularization
 *
 * 
 */
        template <EnumRegularization Regularization, typename ItSource, typename ItDelta, typename ItTargetGradient, typename ItGradient, typename ItWeight>
            void update (ItSource itSource, ItSource itSourceEnd, 
                         ItDelta itTargetDeltaBegin, ItDelta itTargetDeltaEnd, 
                         ItTargetGradient itTargetGradientBegin, 
                         ItGradient itGradient, 
                         ItWeight itWeight, double weightDecay)
        {
            // ! the factor weightDecay has to be already scaled by 1/n where n is the number of weights
            while (itSource != itSourceEnd)
            {
                auto itTargetDelta = itTargetDeltaBegin;
                auto itTargetGradient = itTargetGradientBegin;
                while (itTargetDelta != itTargetDeltaEnd)
                {
                    (*itGradient) -= + (*itTargetDelta) * (*itSource) * (*itTargetGradient) + computeRegularization<Regularization>(*itWeight,weightDecay);
                    ++itTargetDelta; ++itTargetGradient; ++itGradient; ++itWeight;
                }
                ++itSource; 
            }
        }






#define USELOCALWEIGHTS 1



/*! \brief implementation of the steepest gradient descent algorithm
 *
 * Can be used with multithreading (i.e. "HogWild!" style); see call in trainCycle
 */
        template <typename Function, typename Weights, typename PassThrough>
            double Steepest::operator() (Function& fitnessFunction, Weights& weights, PassThrough& passThrough) 
        {
            size_t numWeights = weights.size ();
            std::vector<double> gradients (numWeights, 0.0);
            std::vector<double> localWeights (begin (weights), end (weights));

            double E = 1e10;
            if (m_prevGradients.size () != numWeights)
            {
                m_prevGradients.clear ();
                m_prevGradients.assign (weights.size (), 0);
            }

            bool success = true;
            size_t currentRepetition = 0;
            while (success)
            {
                if (currentRepetition >= m_repetitions)
                    break;

                gradients.assign (numWeights, 0.0);

                // --- nesterov momentum ---
                // apply momentum before computing the new gradient
                auto itPrevG = begin (m_prevGradients);
                auto itPrevGEnd = end (m_prevGradients);
                auto itLocWeight = begin (localWeights);
                for (; itPrevG != itPrevGEnd; ++itPrevG)
                {
                    (*itPrevG) *= m_beta;
                    (*itLocWeight) += (*itPrevG);
                }

                E = fitnessFunction (passThrough, localWeights, gradients);
//            plotGradients (gradients);

                double alpha = gaussDouble (m_alpha, m_alpha/2.0);
//            double alpha = m_alpha;

                auto itG = begin (gradients);
                auto itGEnd = end (gradients);
                itPrevG = begin (m_prevGradients);
                double maxGrad = 0.0;
                for (; itG != itGEnd; ++itG, ++itPrevG)
                {
                    double currGrad = (*itG);
                    double prevGrad = (*itPrevG);
                    currGrad *= alpha;
                
                    //(*itPrevG) = m_beta * (prevGrad + currGrad);
                    currGrad += prevGrad;
                    (*itG) = currGrad;
                    (*itPrevG) = currGrad;
                    
                    if (std::fabs (currGrad) > maxGrad)
                        maxGrad = currGrad;
                }

                if (maxGrad > 1)
                {
                    m_alpha /= 2;
                    std::cout << "learning rate reduced to " << m_alpha << std::endl;
                    std::for_each (weights.begin (), weights.end (), [maxGrad](double& w)
                                   {
                                       w /= maxGrad;
                                   });
                    m_prevGradients.clear ();
                }
                else
                {
                    auto itW = std::begin (weights);
                    std::for_each (std::begin (gradients), std::end (gradients), [&itW](double& g)
                                   {
                                       *itW += g;
                                       ++itW;
                                   });
                }

                ++currentRepetition;
            }
            return E;
        }


















/*! \brief sum of squares error function
 *
 * 
 */
        template <typename ItOutput, typename ItTruth, typename ItDelta, typename InvFnc>
            double sumOfSquares (ItOutput itOutputBegin, ItOutput itOutputEnd, ItTruth itTruthBegin, ItTruth /*itTruthEnd*/, ItDelta itDelta, ItDelta itDeltaEnd, InvFnc invFnc, double patternWeight) 
        {
            double errorSum = 0.0;

            // output - truth
            ItTruth itTruth = itTruthBegin;
            bool hasDeltas = (itDelta != itDeltaEnd);
            for (ItOutput itOutput = itOutputBegin; itOutput != itOutputEnd; ++itOutput, ++itTruth)
            {
//	assert (itTruth != itTruthEnd);
                double output = (*itOutput);
                double error = output - (*itTruth);
                if (hasDeltas)
                {
                    (*itDelta) = (*invFnc.get ()) (output) * error * patternWeight;
                    ++itDelta; 
                }
                errorSum += error*error  * patternWeight;
            }

            return 0.5*errorSum;
        }



/*! \brief cross entropy error function
 *
 * 
 */
        template <typename ItProbability, typename ItTruth, typename ItDelta, typename ItInvActFnc>
            double crossEntropy (ItProbability itProbabilityBegin, ItProbability itProbabilityEnd, ItTruth itTruthBegin, ItTruth /*itTruthEnd*/, ItDelta itDelta, ItDelta itDeltaEnd, ItInvActFnc /*itInvActFnc*/, double patternWeight) 
        {
            bool hasDeltas = (itDelta != itDeltaEnd);
    
            double errorSum = 0.0;
            for (ItProbability itProbability = itProbabilityBegin; itProbability != itProbabilityEnd; ++itProbability)
            {
                double probability = *itProbability;
                double truth = *itTruthBegin;
                /* truth = truth < 0.1 ? 0.1 : truth; */
                /* truth = truth > 0.9 ? 0.9 : truth; */
                truth = truth < 0.5 ? 0.1 : 0.9;
                if (hasDeltas)
                {
                    double delta = probability - truth;
                    (*itDelta) = delta*patternWeight;
//	    (*itDelta) = (*itInvActFnc)(probability) * delta * patternWeight;
                    ++itDelta;
                }
                double error (0);
                if (probability == 0) // protection against log (0)
                {
                    if (truth >= 0.5)
                        error += 1.0;
                }
                else if (probability == 1)
                {
                    if (truth < 0.5)
                        error += 1.0;
                }
                else
                    error += - (truth * log (probability) + (1.0-truth) * log (1.0-probability)); // cross entropy function
                errorSum += error * patternWeight;
        
            }
            return errorSum;
        }




/*! \brief soft-max-cross-entropy error function (for mutual exclusive cross-entropy)
 *
 * 
 */
        template <typename ItOutput, typename ItTruth, typename ItDelta, typename ItInvActFnc>
            double softMaxCrossEntropy (ItOutput itProbabilityBegin, ItOutput itProbabilityEnd, ItTruth itTruthBegin, ItTruth /*itTruthEnd*/, ItDelta itDelta, ItDelta itDeltaEnd, ItInvActFnc /*itInvActFnc*/, double patternWeight) 
        {
            double errorSum = 0.0;

            bool hasDeltas = (itDelta != itDeltaEnd);
            // output - truth
            ItTruth itTruth = itTruthBegin;
            for (auto itProbability = itProbabilityBegin; itProbability != itProbabilityEnd; ++itProbability, ++itTruth)
            {
//	assert (itTruth != itTruthEnd);
                double probability = (*itProbability);
                double truth = (*itTruth);
                if (hasDeltas)
                {
                    (*itDelta) = probability - truth;
//	    (*itDelta) = (*itInvActFnc)(sm) * delta * patternWeight;
                    ++itDelta; //++itInvActFnc;
                }
                double error (0);

                error += truth * log (probability);
                errorSum += error;
            }

            return -errorSum * patternWeight;
        }









/*! \brief compute the weight decay for regularization (L1 or L2)
 *
 * 
 */
        template <typename ItWeight>
            double weightDecay (double error, ItWeight itWeight, ItWeight itWeightEnd, double factorWeightDecay, EnumRegularization eRegularization)
        {
            if (eRegularization == EnumRegularization::L1)
            {
                // weight decay (regularization)
                double w = 0;
                size_t n = 0;
                for (; itWeight != itWeightEnd; ++itWeight, ++n)
                {
                    double weight = (*itWeight);
                    w += std::fabs (weight);
                }
                return error + 0.5 * w * factorWeightDecay / n;
            }
            else if (eRegularization == EnumRegularization::L2)
            {
                // weight decay (regularization)
                double w = 0;
                size_t n = 0;
                for (; itWeight != itWeightEnd; ++itWeight, ++n)
                {
                    double weight = (*itWeight);
                    w += weight*weight;
                }
                return error + 0.5 * w * factorWeightDecay / n;
            }
            else
                return error;
        }














/*! \brief apply the weights in forward direction of the DNN
 *
 * 
 */
        template <typename LAYERDATA>
            void forward (const LAYERDATA& prevLayerData, LAYERDATA& currLayerData)
        {
            if (prevLayerData.hasDropOut ())
            {        
                applyWeights (prevLayerData.valuesBegin (), prevLayerData.valuesEnd (), 
                              currLayerData.weightsBegin (), 
                              currLayerData.valuesBegin (), currLayerData.valuesEnd (),
                              prevLayerData.dropOut ());
            }
            else
            {
                applyWeights (prevLayerData.valuesBegin (), prevLayerData.valuesEnd (), 
                              currLayerData.weightsBegin (), 
                              currLayerData.valuesBegin (), currLayerData.valuesEnd ());
            }
        }

/*! \brief apply weights (and functions) in forward direction and compute the gradients
 *
 * 
 */
        template <typename LAYERDATA>
            void forward_training (const LAYERDATA& prevLayerData, LAYERDATA& currLayerData)
        {
            if (prevLayerData.hasDropOut ())
            {        
                applyWeights (prevLayerData.valuesBegin (), prevLayerData.valuesEnd (), 
                              currLayerData.weightsBegin (), 
                              currLayerData.valuesBegin (), currLayerData.valuesEnd (),
                              prevLayerData.dropOut ());
            }
            else
            {
                applyWeights (prevLayerData.valuesBegin (), prevLayerData.valuesEnd (), 
                              currLayerData.weightsBegin (), 
                              currLayerData.valuesBegin (), currLayerData.valuesEnd ());
            }
        }


/*! \brief backward application of the weights (back-propagation of the error)
 *
 * 
 */
        template <typename LAYERDATA>
            void backward (LAYERDATA& prevLayerData, LAYERDATA& currLayerData)
        {
            if (prevLayerData.hasDropOut ())
            {
                applyWeightsBackwards (currLayerData.deltasBegin (), currLayerData.deltasEnd (), 
                                       currLayerData.weightsBegin (), 
                                       prevLayerData.deltasBegin (), prevLayerData.deltasEnd (),
                                       prevLayerData.dropOut ());
            }
            else
            {
                applyWeightsBackwards (currLayerData.deltasBegin (), currLayerData.deltasEnd (), 
                                       currLayerData.weightsBegin (), 
                                       prevLayerData.deltasBegin (), prevLayerData.deltasEnd ());
            }
        }



/*! \brief update the node values
 *
 * 
 */
        template <typename LAYERDATA>
            void update (const LAYERDATA& prevLayerData, LAYERDATA& currLayerData, double factorWeightDecay, EnumRegularization regularization)
        {
            // ! the "factorWeightDecay" has already to be scaled by 1/n where n is the number of weights
            if (factorWeightDecay != 0.0) // has weight regularization
                if (regularization == EnumRegularization::L1)  // L1 regularization ( sum(|w|) )
                {
                    update<EnumRegularization::L1> (prevLayerData.valuesBegin (), prevLayerData.valuesEnd (), 
                                                    currLayerData.deltasBegin (), currLayerData.deltasEnd (), 
                                                    currLayerData.valueGradientsBegin (), currLayerData.gradientsBegin (), 
                                                    currLayerData.weightsBegin (), factorWeightDecay);
                }
                else if (regularization == EnumRegularization::L2) // L2 regularization ( sum(w^2) )
                {
                    update<EnumRegularization::L2> (prevLayerData.valuesBegin (), prevLayerData.valuesEnd (), 
                                                    currLayerData.deltasBegin (), currLayerData.deltasEnd (), 
                                                    currLayerData.valueGradientsBegin (), currLayerData.gradientsBegin (), 
                                                    currLayerData.weightsBegin (), factorWeightDecay);
                }
                else 
                {
                    update (prevLayerData.valuesBegin (), prevLayerData.valuesEnd (), 
                            currLayerData.deltasBegin (), currLayerData.deltasEnd (), 
                            currLayerData.valueGradientsBegin (), currLayerData.gradientsBegin ());
                }
    
            else
            { // no weight regularization
                update (prevLayerData.valuesBegin (), prevLayerData.valuesEnd (), 
                        currLayerData.deltasBegin (), currLayerData.deltasEnd (), 
                        currLayerData.valueGradientsBegin (), currLayerData.gradientsBegin ());
            }
        }












/*! \brief compute the drop-out-weight factor
 *
 * when using drop-out a fraction of the nodes is turned off at each cycle of the computation
 * once all nodes are turned on again (for instances when the test samples are evaluated), 
 * the weights have to be adjusted to account for the different number of active nodes
 * this function computes the factor and applies it to the weights
 */
        template <typename WeightsType, typename DropProbabilities>
            void Net::dropOutWeightFactor (WeightsType& weights,
                                           const DropProbabilities& drops, 
                                           bool inverse)
        {
            if (drops.empty () || weights.empty ())
                return;

            auto itWeight = std::begin (weights);
            auto itWeightEnd = std::end (weights);
            auto itDrop = std::begin (drops);
            auto itDropEnd = std::end (drops);
            size_t numNodesPrev = inputSize ();
            double dropFractionPrev = *itDrop;
            ++itDrop;

            for (auto& layer : layers ())
            {
                if (itDrop == itDropEnd)
                    break;

                size_t numNodes = layer.numNodes ();

                double dropFraction = *itDrop;
                double pPrev = 1.0 - dropFractionPrev;
                double p = 1.0 - dropFraction;
                p *= pPrev;

                if (inverse)
                {
                    p = 1.0/p;
                }
                size_t _numWeights = layer.numWeights (numNodesPrev);
                for (size_t iWeight = 0; iWeight < _numWeights; ++iWeight)
                {
                    if (itWeight == itWeightEnd)
                        break;
                
                    *itWeight *= p;
                    ++itWeight;
                }
                numNodesPrev = numNodes;
                dropFractionPrev = dropFraction;
                ++itDrop;
            }
        }



        
    

/*! \brief execute the training until convergence emerges
 *
 * \param weights the container with the weights (synapses)
 * \param trainPattern the pattern for the training
 * \param testPattern the pattern for the testing
 * \param minimizer the minimizer (e.g. steepest gradient descent) to be used
 * \param settings the settings for the training (e.g. multithreading or not, regularization etc.)
 */
        template <typename Minimizer>
            double Net::train (std::vector<double>& weights, 
                               std::vector<Pattern>& trainPattern, 
                               const std::vector<Pattern>& testPattern, 
                               Minimizer& minimizer, Settings& settings)
        {
//        std::cout << "START TRAINING" << std::endl;
            settings.startTrainCycle ();

            settings.pads (4);
            settings.create ("trainErrors", 100, 0, 100, 100, 0,1);
            settings.create ("testErrors", 100, 0, 100, 100, 0,1);

            size_t cycleCount = 0;
            size_t testCycleCount = 0;
            double testError = 1e20;
            double trainError = 1e20;
            size_t dropOutChangeCount = 0;

            DropContainer dropContainer;
            DropContainer dropContainerTest;
            const std::vector<double>& dropFractions = settings.dropFractions ();
            bool isWeightsForDrop = false;

        
            // until convergence
            do
            {
                ++cycleCount;

                // if dropOut enabled
                size_t dropIndex = 0;
                if (!dropFractions.empty () && dropOutChangeCount % settings.dropRepetitions () == 0)
                {
                    // fill the dropOut-container
                    dropContainer.clear ();
                    size_t numNodes = inputSize ();
                    double dropFraction = 0.0;
                    dropFraction = dropFractions.at (dropIndex);
                    ++dropIndex;
                    fillDropContainer (dropContainer, dropFraction, numNodes);
                    for (auto itLayer = begin (m_layers), itLayerEnd = end (m_layers); itLayer != itLayerEnd; ++itLayer, ++dropIndex)
                    {
                        auto& layer = *itLayer;
                        numNodes = layer.numNodes ();
                        // how many nodes have to be dropped
                        dropFraction = 0.0;
                        if (dropFractions.size () > dropIndex)
                            dropFraction = dropFractions.at (dropIndex);
                    
                        fillDropContainer (dropContainer, dropFraction, numNodes);
                    }
                    isWeightsForDrop = true;
                }

                // execute training cycle
                trainError = trainCycle (minimizer, weights, begin (trainPattern), end (trainPattern), settings, dropContainer);
	    

                // check if we execute a test
                bool hasConverged = false;
                if (testCycleCount % settings.testRepetitions () == 0)
                {
                    if (isWeightsForDrop)
                    {
                        dropOutWeightFactor (weights, dropFractions);
                        isWeightsForDrop = false;
                    }

                    testError = 0;
                    //double weightSum = 0;
                    settings.startTestCycle ();
                    if (settings.useMultithreading ())
                    {
                        size_t numThreads = std::thread::hardware_concurrency ();
                        size_t patternPerThread = testPattern.size () / numThreads;
                        std::vector<Batch> batches;
                        auto itPat = testPattern.begin ();
                        auto itPatEnd = testPattern.end ();
                        for (size_t idxThread = 0; idxThread < numThreads-1; ++idxThread)
                        {
                            batches.push_back (Batch (itPat, itPat + patternPerThread));
                            itPat += patternPerThread;
                        }
                        batches.insert (batches.end (), Batch (itPat, itPatEnd));

                        std::vector<std::future<std::tuple<double,std::vector<double>>>> futures;
                        for (auto& batch : batches)
                        {
                            // -------------------- execute each of the batch ranges on a different thread -------------------------------
                            futures.push_back (
                                std::async (std::launch::async, [&]() 
                                            {
                                                std::vector<double> localOutput;
                                                std::tuple<Settings&, Batch&, DropContainer&> passThrough (settings, batch, dropContainerTest);
                                                double testBatchError = (*this) (passThrough, weights, ModeOutput::FETCH, localOutput);
                                                return std::make_tuple (testBatchError, localOutput);
                                            })
                                );
                        }

                        for (auto& f : futures)
                        {
                            std::tuple<double,std::vector<double>> result = f.get ();
                            testError += std::get<0>(result) / batches.size ();
                            std::vector<double> output = std::get<1>(result);
                            if (output.size () == testPattern.size ())
                            {
                                auto it = begin (testPattern);
                                for (double out : output)
                                {
                                    settings.testSample (0, out, (*it).output ().at (0), (*it).weight ());
                                    ++it;
                                }
                            }
                        }
                    
                    }
                    else
                    {
                        std::vector<double> output;
                        for (auto it = begin (testPattern), itEnd = end (testPattern); it != itEnd; ++it)
                        {
                            const Pattern& p = (*it);
                            double weight = p.weight ();
                            Batch batch (it, it+1);
                            output.clear ();
                            std::tuple<Settings&, Batch&, DropContainer&> passThrough (settings, batch, dropContainerTest);
                            double testPatternError = (*this) (passThrough, weights, ModeOutput::FETCH, output);
                            if (output.size () == 1)
                            {
                                /* std::vector<double> out = (*this).compute (p.input (), weights); */
                                /* assert (output.at (0) == out.at (0)); */
                                settings.testSample (testPatternError, output.at (0), p.output ().at (0), weight);
                            }
                            //weightSum += fabs (weight);
                            //testError += testPatternError*weight;
                            testError += testPatternError;
                        }
                        testError /= testPattern.size ();
                    }
                    settings.endTestCycle ();
//                    testError /= weightSum;

                    settings.computeResult (*this, weights);

                    hasConverged = settings.hasConverged (testError);
                    if (!hasConverged && !isWeightsForDrop)
                    {
                        dropOutWeightFactor (weights, dropFractions, true); // inverse
                        isWeightsForDrop = true;
                    }
                }
                ++testCycleCount;
                ++dropOutChangeCount;


                static double x = -1.0;
                x += 1.0;
//            settings.resetPlot ("errors");
                settings.addPoint ("trainErrors", cycleCount, trainError);
                settings.addPoint ("testErrors", cycleCount, testError);
                settings.plot ("trainErrors", "C", 1, kBlue);
                settings.plot ("testErrors", "C", 1, kMagenta);



                if (hasConverged)
                    break;


                TString convText = Form( "<D^2> (train/test/epoch/conv/maxConv): %.4g/%.4g/%d/%d/%d",
                                         trainError,
                                         testError,
                                         (int)cycleCount,
                                         (int)settings.convergenceCount (),
                                         (int)settings.maxConvergenceCount ());
                double progress = 100*(double)settings.maxConvergenceCount () /(double)settings.convergenceSteps ();
                settings.cycle (progress, convText);
            }
            while (true);
            settings.endTrainCycle (trainError);
        
            TString convText = Form( "<D^2> (train/test/epoch): %.4g/%.4g/%d", trainError, testError, (int)cycleCount);
            double progress = 100*(double)settings.maxConvergenceCount() /(double)settings.convergenceSteps ();
            settings.cycle (progress, convText);

            return testError;
        }



/*! \brief execute a single training cycle
 *
 * uses multithreading if turned on
 *
 * \param minimizer the minimizer to be used (e.g. SGD)
 * \param weights the weight container with all the synapse weights
 * \param itPatternBegin begin of the pattern container
 * \parama itPatternEnd the end of the pattern container
 * \param settings the settings for this training (e.g. multithreading or not, regularization, etc.)
 * \param dropContainer the data for dropping-out nodes (regularization technique)
 */
        template <typename Iterator, typename Minimizer>
            inline double Net::trainCycle (Minimizer& minimizer, std::vector<double>& weights, 
                                           Iterator itPatternBegin, Iterator itPatternEnd, Settings& settings, DropContainer& dropContainer)
        {
            double error = 0.0;
            size_t numPattern = std::distance (itPatternBegin, itPatternEnd);
            size_t numBatches = numPattern/settings.batchSize ();
            size_t numBatches_stored = numBatches;

            std::random_shuffle (itPatternBegin, itPatternEnd);
            Iterator itPatternBatchBegin = itPatternBegin;
            Iterator itPatternBatchEnd = itPatternBatchBegin;

            // create batches
            std::vector<Batch> batches;
            while (numBatches > 0)
            {
                std::advance (itPatternBatchEnd, settings.batchSize ());
                batches.push_back (Batch (itPatternBatchBegin, itPatternBatchEnd));
                itPatternBatchBegin = itPatternBatchEnd;
                --numBatches;
            }

            // add the last pattern to the last batch
            if (itPatternBatchEnd != itPatternEnd)
                batches.push_back (Batch (itPatternBatchEnd, itPatternEnd));


            ///< turn on multithreading if requested
            if (settings.useMultithreading ())
            {
                // -------------------- divide the batches into bunches for each thread --------------
                size_t numThreads = std::thread::hardware_concurrency ();
                size_t batchesPerThread = batches.size () / numThreads;
                typedef std::vector<Batch>::iterator batch_iterator;
                std::vector<std::pair<batch_iterator,batch_iterator>> batchVec;
                batch_iterator itBatchBegin = std::begin (batches);
                batch_iterator itBatchCurrEnd = std::begin (batches);
                batch_iterator itBatchEnd = std::end (batches);
                for (size_t iT = 0; iT < numThreads; ++iT)
                {
                    if (iT == numThreads-1)
                        itBatchCurrEnd = itBatchEnd;
                    else
                        std::advance (itBatchCurrEnd, batchesPerThread);
                    batchVec.push_back (std::make_pair (itBatchBegin, itBatchCurrEnd));
                    itBatchBegin = itBatchCurrEnd;
                }
        
                // -------------------- loop  over batches -------------------------------------------
                std::vector<std::future<double>> futures;
                for (auto& batchRange : batchVec)
                {
                    // -------------------- execute each of the batch ranges on a different thread -------------------------------
                    futures.push_back (
                        std::async (std::launch::async, [&]() 
                                    {
                                        double localError = 0.0;
                                        for (auto it = batchRange.first, itEnd = batchRange.second; it != itEnd; ++it)
                                        {
                                            Batch& batch = *it;
                                            std::tuple<Settings&, Batch&, DropContainer&> settingsAndBatch (settings, batch, dropContainer);
                                            localError += minimizer ((*this), weights, settingsAndBatch); /// call the minimizer
                                        }
                                        return localError;
                                    })
                        );
                }

                for (auto& f : futures)
                    error += f.get ();
            }
            else
            {
                for (auto& batch : batches)
                {
                    std::tuple<Settings&, Batch&, DropContainer&> settingsAndBatch (settings, batch, dropContainer);
                    error += minimizer ((*this), weights, settingsAndBatch);
                }
            }
        
            numBatches_stored = std::max (numBatches_stored, size_t(1)); /// normalize the error
            error /= numBatches_stored;
            settings.testIteration ();
    
            return error;
        }





/*! \brief compute the neural net
 *
 * \param input the input data
 * \param weights the weight data
 */
        template <typename Weights>
            std::vector<double> Net::compute (const std::vector<double>& input, const Weights& weights) const
        {
            std::vector<LayerData> layerData;
            layerData.reserve (m_layers.size ()+1);
            auto itWeight = begin (weights);
            auto itInputBegin = begin (input);
            auto itInputEnd = end (input);
            layerData.push_back (LayerData (itInputBegin, itInputEnd));
            size_t numNodesPrev = input.size ();
            for (auto& layer: m_layers)
            {
                layerData.push_back (LayerData (layer.numNodes (), itWeight, 
                                                layer.activationFunction (),
                                                layer.modeOutputValues ()));
                size_t _numWeights = layer.numWeights (numNodesPrev);
                itWeight += _numWeights;
                numNodesPrev = layer.numNodes ();
            }
	    

            // --------- forward -------------
            size_t idxLayer = 0, idxLayerEnd = m_layers.size ();
            for (; idxLayer < idxLayerEnd; ++idxLayer)
            {
                LayerData& prevLayerData = layerData.at (idxLayer);
                LayerData& currLayerData = layerData.at (idxLayer+1);
		
                forward (prevLayerData, currLayerData);
                applyFunctions (currLayerData.valuesBegin (), currLayerData.valuesEnd (), currLayerData.activationFunction ());
            }

            // ------------- fetch output ------------------
            if (TMVA::DNN::isFlagSet (ModeOutputValues::DIRECT, layerData.back ().outputMode ()))
            {
                std::vector<double> output;
                output.assign (layerData.back ().valuesBegin (), layerData.back ().valuesEnd ());
                return output;
            }
            std::vector<double> output (layerData.back ().probabilities ());
            return output;
        }


        template <typename Weights, typename PassThrough>
            double Net::operator() (PassThrough& settingsAndBatch, const Weights& weights) const
        {
            std::vector<double> nothing; // empty gradients; no backpropagation is done, just forward
            assert (numWeights () == weights.size ());
            double error = forward_backward(m_layers, settingsAndBatch, std::begin (weights), std::begin (nothing), std::end (nothing), 10000, nothing, false);
            return error;
        }

        template <typename Weights, typename PassThrough, typename OutContainer>
            double Net::operator() (PassThrough& settingsAndBatch, const Weights& weights, ModeOutput /*eFetch*/, OutContainer& outputContainer) const
        {
            std::vector<double> nothing; // empty gradients; no backpropagation is done, just forward
            assert (numWeights () == weights.size ());
            double error = forward_backward(m_layers, settingsAndBatch, std::begin (weights), std::begin (nothing), std::end (nothing), 10000, outputContainer, true);
            return error;
        }

    
        template <typename Weights, typename Gradients, typename PassThrough>
            double Net::operator() (PassThrough& settingsAndBatch, const Weights& weights, Gradients& gradients) const
        {
            std::vector<double> nothing;
            assert (numWeights () == weights.size ());
            assert (weights.size () == gradients.size ());
            double error = forward_backward(m_layers, settingsAndBatch, std::begin (weights), std::begin (gradients), std::end (gradients), 0, nothing, false);
            return error;
        }

        template <typename Weights, typename Gradients, typename PassThrough, typename OutContainer>
            double Net::operator() (PassThrough& settingsAndBatch, const Weights& weights, Gradients& gradients, ModeOutput eFetch, OutContainer& outputContainer) const
        {
            assert (numWeights () == weights.size ());
            assert (weights.size () == gradients.size ());
            double error = forward_backward(m_layers, settingsAndBatch, std::begin (weights), std::begin (gradients), std::end (gradients), 0, outputContainer, true);
            return error;
        }





/*! \brief forward propagation and backward propagation
 *
 * 
 */
        template <typename LayerContainer, typename PassThrough, typename ItWeight, typename ItGradient, typename OutContainer>
            double Net::forward_backward (LayerContainer& _layers, PassThrough& settingsAndBatch, 
                                          ItWeight itWeightBegin, 
                                          ItGradient itGradientBegin, ItGradient itGradientEnd, 
                                          size_t trainFromLayer, 
                                          OutContainer& outputContainer, bool fetchOutput) const
        {
            Settings& settings = std::get<0>(settingsAndBatch);
            Batch& batch = std::get<1>(settingsAndBatch);
            DropContainer& dropContainer = std::get<2>(settingsAndBatch);

            bool doBatchNormalization = settings.doBatchNormalization ();
            bool usesDropOut = !dropContainer.empty ();

            LayerData::const_dropout_iterator itDropOut;
            if (usesDropOut)
                itDropOut = std::begin (dropContainer);
        
            if (_layers.empty ())
            {
                std::cout << "no layers in this net" << std::endl;
                throw std::string ("no layers in this net");
            }


            double sumError = 0.0;
            double sumWeights = 0.0;	// -------------

            // ----------- create layer data -------------------------------------------------------
            assert (_layers.back ().numNodes () == outputSize ());
            size_t totalNumWeights = 0;
            std::vector<std::vector<LayerData>> layerPatternData;
            layerPatternData.reserve (_layers.size ()+1);
            ItWeight itWeight = itWeightBegin;
            ItGradient itGradient = itGradientBegin;
            size_t numNodesPrev = inputSize ();
            typename Pattern::const_iterator itInputBegin;
            typename Pattern::const_iterator itInputEnd;

            // --------------------- prepare layer data for input layer ----------------------------
            layerPatternData.push_back (std::vector<LayerData>());
            for (const Pattern& _pattern : batch)
            {
                std::vector<LayerData>& layerData = layerPatternData.back ();
                layerData.push_back (LayerData (numNodesPrev));

                itInputBegin = _pattern.beginInput ();
                itInputEnd = _pattern.endInput ();
                layerData.back ().setInput (itInputBegin, itInputEnd);
            
                if (usesDropOut)
                {
                    layerData.back ().setDropOut (itDropOut);
                }
            }
            if (usesDropOut)
            {
                itDropOut += _layers.back ().numNodes ();
            }

            // ---------------- prepare subsequent layers ---------------------------------------------
            // for each of the layers
            for (auto& layer: _layers)
            {
                layerPatternData.push_back (std::vector<LayerData>());
                // for each pattern, prepare a layerData
                for (const Pattern& _pattern : batch)
                {
                    std::vector<LayerData>& layerData = layerPatternData.back ();
                    //layerData.push_back (LayerData (numNodesPrev));

                    if (itGradientBegin == itGradientEnd)
                        layerData.push_back (LayerData (layer.numNodes (), itWeight, 
                                                        layer.activationFunction (),
                                                        layer.modeOutputValues ()));
                    else
                        layerData.push_back (LayerData (layer.numNodes (), itWeight, itGradient, 
                                                        layer.activationFunction (),
                                                        layer.inverseActivationFunction (),
                                                        layer.modeOutputValues ()));

                    if (usesDropOut)
                    {
                        layerData.back ().setDropOut (itDropOut);
                    }
                }

                if (usesDropOut)
                {
                    itDropOut += layer.numNodes ();
                }
                size_t _numWeights = layer.numWeights (numNodesPrev);
                totalNumWeights += _numWeights;
                itWeight += _numWeights;
                itGradient += _numWeights;
                numNodesPrev = layer.numNodes ();
            }
            assert (totalNumWeights > 0);



            // ---------------------------------- loop over layers and pattern -------------------------------------------------------
            for (size_t idxLayer = 0, idxLayerEnd = layerPatternData.size (); idxLayer < idxLayerEnd-1; ++idxLayer) //std::vector<LayerData>& layerPattern : layerPatternData)
            {
                bool doTraining = idxLayer >= trainFromLayer;

                // get layer-pattern data for this and the corresponding one from the next layer
                std::vector<LayerData>& prevLayerPatternData = layerPatternData.at (idxLayer);
                std::vector<LayerData>& currLayerPatternData = layerPatternData.at (idxLayer+1);

                size_t numPattern = prevLayerPatternData.size ();

                std::vector<MeanVariance> means (_layers.at (idxLayer).numNodes ());
                // ---------------- loop over layerDatas of pattern compute forward ----------------------------
                for (size_t idxPattern = 0; idxPattern < numPattern; ++idxPattern)
                {
                    const LayerData& prevLayerData = prevLayerPatternData.at (idxPattern);
                    LayerData& currLayerData = currLayerPatternData.at (idxPattern);
                
            
                    if (doTraining)
                        forward_training (prevLayerData, currLayerData);
                    else
                        forward (prevLayerData, currLayerData);


                    // -------- compute batch mean and variance if batch normalization is turned on ------------------
                    if (doBatchNormalization && doTraining)
                    {
//                    means.at (idxPattern).add (*(prevLayerData.valuesBegin ()+idxPattern));
                    }
                }

                // ---------------- do batch normalization ----------------------------
                if (doBatchNormalization)
                {
                    if (doTraining) // take means and variances from batch
                    {
                        for (size_t idxPattern = 0; idxPattern < numPattern; ++idxPattern)
                        {
                        }
                    }
                    else // take average mean and variance for batch normalization
                    {
                    }
                }
            
                // ---------------- loop over layerDatas of pattern apply non-linearities ----------------------------
                for (size_t idxPattern = 0; idxPattern < numPattern; ++idxPattern)
                {
                    const LayerData& prevLayerData = prevLayerPatternData.at (idxPattern);
                    LayerData& currLayerData = currLayerPatternData.at (idxPattern);
                
                    if (doTraining)
                        applyFunctions (currLayerData.valuesBegin (), currLayerData.valuesEnd (), currLayerData.activationFunction (), 
                                        currLayerData.inverseActivationFunction (), currLayerData.valueGradientsBegin ());
                    else
                        applyFunctions (currLayerData.valuesBegin (), currLayerData.valuesEnd (), currLayerData.activationFunction ());
                }

            }




        
            // ------------- fetch output ------------------
            if (fetchOutput)
            {
                for (LayerData& lastLayerData : layerPatternData.back ())
                {
                    ModeOutputValues eModeOutput = lastLayerData.outputMode ();
                    if (TMVA::DNN::isFlagSet (ModeOutputValues::DIRECT, eModeOutput))
                    {
                        outputContainer.insert (outputContainer.end (), lastLayerData.valuesBegin (), lastLayerData.valuesEnd ());
                    }
                    else if (TMVA::DNN::isFlagSet (ModeOutputValues::SIGMOID, eModeOutput) ||
                             TMVA::DNN::isFlagSet (ModeOutputValues::SOFTMAX, eModeOutput))
                    {
                        const auto& probs = lastLayerData.probabilities ();
                        outputContainer.insert (outputContainer.end (), probs.begin (), probs.end ());
                    }
                    else
                        assert (false);
                }
            }


            // ------------- error computation -------------
            std::vector<LayerData>& lastLayerData = layerPatternData.back ();

            bool doTraining = layerPatternData.size () > trainFromLayer;

            typename std::vector<LayerData>::iterator itLayerData    = lastLayerData.begin ();
            typename std::vector<LayerData>::iterator itLayerDataEnd = lastLayerData.end ();

            typename std::vector<Pattern>::const_iterator itPattern = batch.begin ();
            typename std::vector<Pattern>::const_iterator itPatternEnd = batch.end ();

            size_t idxPattern = 0;
            for ( ; itPattern != itPatternEnd; ++itPattern, ++itLayerData)
            {
                ++idxPattern;
                
                // compute E and the deltas of the computed output and the true output
                LayerData& layerData = (*itLayerData);
                const Pattern& _pattern = (*itPattern);
                itWeight = itWeightBegin;
                double error = errorFunction (layerData, _pattern.output (), 
                                              itWeight, itWeight + totalNumWeights, 
                                              _pattern.weight (), settings.factorWeightDecay (),
                                              settings.regularization ());
                sumWeights += fabs (_pattern.weight ());
                sumError += error;
            }
            
            if (doTraining) // training
            {
                // ------------- backpropagation -------------
                size_t idxLayer = layerPatternData.size ();
                for (auto itLayerPatternData = layerPatternData.rbegin (), itLayerPatternDataBegin = layerPatternData.rend ();
                     itLayerPatternData != itLayerPatternDataBegin; ++itLayerPatternData)
                {
                    --idxLayer;
                    if (idxLayer <= trainFromLayer) // no training
                        break;

                    std::vector<LayerData>& currLayerDataColl = *(itLayerPatternData);
                    std::vector<LayerData>& prevLayerDataColl = *(itLayerPatternData+1);
                
                    idxPattern = 0;
                    for (typename std::vector<LayerData>::iterator itCurrLayerData = begin (currLayerDataColl), itCurrLayerDataEnd = end (currLayerDataColl),
                             itPrevLayerData = begin (prevLayerDataColl), itPrevLayerDataEnd = end (prevLayerDataColl);
                         itCurrLayerData != itCurrLayerDataEnd; ++itCurrLayerData, ++itPrevLayerData, ++idxPattern)
                    {
                        LayerData& currLayerData = (*itCurrLayerData);
                        LayerData& prevLayerData = *(itPrevLayerData);

                        backward (prevLayerData, currLayerData);

                        // the factorWeightDecay has to be scaled by 1/n where n is the number of weights (synapses)
                        // because L1 and L2 regularization
                        //
                        //  http://neuralnetworksanddeeplearning.com/chap3.html#overfitting_and_regularization
                        //
                        // L1 : -factorWeightDecay*sgn(w)/numWeights
                        // L2 : -factorWeightDecay/numWeights
                        update (prevLayerData, currLayerData, settings.factorWeightDecay ()/totalNumWeights, settings.regularization ());
                    }
                }
            }
        
            double batchSize = std::distance (std::begin (batch), std::end (batch));
            for (auto it = itGradientBegin; it != itGradientEnd; ++it)
                (*it) /= batchSize;


            sumError /= sumWeights;
            return sumError;
        }



/*! \brief initialization of the weights
 *
 * 
 */
        template <typename OutIterator>
            void Net::initializeWeights (WeightInitializationStrategy eInitStrategy, OutIterator itWeight)
        {
            if (eInitStrategy == WeightInitializationStrategy::XAVIER)
            {
                // input and output properties
                int numInput = inputSize ();

                // compute variance and mean of input and output
                //...
	

                // compute the weights
                for (auto& layer: layers ())
                {
                    double nIn = numInput;
                    double stdDev = sqrt (2.0/nIn);
                    for (size_t iWeight = 0, iWeightEnd = layer.numWeights (numInput); iWeight < iWeightEnd; ++iWeight)
                    {
                        (*itWeight) = DNN::gaussDouble (0.0, stdDev); // factor 2.0 for ReLU
                        ++itWeight;
                    }
                    numInput = layer.numNodes ();
                }
                return;
            }

            if (eInitStrategy == WeightInitializationStrategy::XAVIERUNIFORM)
            {
                // input and output properties
                int numInput = inputSize ();

                // compute variance and mean of input and output
                //...
	

                // compute the weights
                for (auto& layer: layers ())
                {
                    double nIn = numInput;
                    double minVal = -sqrt(2.0/nIn);
                    double maxVal = sqrt (2.0/nIn);
                    for (size_t iWeight = 0, iWeightEnd = layer.numWeights (numInput); iWeight < iWeightEnd; ++iWeight)
                    {
                    
                        (*itWeight) = DNN::uniformDouble (minVal, maxVal); // factor 2.0 for ReLU
                        ++itWeight;
                    }
                    numInput = layer.numNodes ();
                }
                return;
            }
        
            if (eInitStrategy == WeightInitializationStrategy::TEST)
            {
                // input and output properties
                int numInput = inputSize ();

                // compute variance and mean of input and output
                //...
	

                // compute the weights
                for (auto& layer: layers ())
                {
//                double nIn = numInput;
                    for (size_t iWeight = 0, iWeightEnd = layer.numWeights (numInput); iWeight < iWeightEnd; ++iWeight)
                    {
                        (*itWeight) = DNN::gaussDouble (0.0, 0.1);
                        ++itWeight;
                    }
                    numInput = layer.numNodes ();
                }
                return;
            }

            if (eInitStrategy == WeightInitializationStrategy::LAYERSIZE)
            {
                // input and output properties
                int numInput = inputSize ();

                // compute variance and mean of input and output
                //...
	

                // compute the weights
                for (auto& layer: layers ())
                {
                    double nIn = numInput;
                    for (size_t iWeight = 0, iWeightEnd = layer.numWeights (numInput); iWeight < iWeightEnd; ++iWeight)
                    {
                        (*itWeight) = DNN::gaussDouble (0.0, sqrt (layer.numWeights (nIn))); // factor 2.0 for ReLU
                        ++itWeight;
                    }
                    numInput = layer.numNodes ();
                }
                return;
            }

        }


    


/*! \brief compute the error function
 *
 * 
 */
        template <typename Container, typename ItWeight>
            double Net::errorFunction (LayerData& layerData,
                                       Container truth,
                                       ItWeight itWeight,
                                       ItWeight itWeightEnd,
                                       double patternWeight,
                                       double factorWeightDecay,
                                       EnumRegularization eRegularization) const
        {
            double error (0);
            switch (m_eErrorFunction)
            {
            case ModeErrorFunction::SUMOFSQUARES:
            {
                error = sumOfSquares (layerData.valuesBegin (), layerData.valuesEnd (), begin (truth), end (truth), 
                                      layerData.deltasBegin (), layerData.deltasEnd (), 
                                      layerData.inverseActivationFunction (), 
                                      patternWeight);
                break;
            }
            case ModeErrorFunction::CROSSENTROPY:
            {
                assert (!TMVA::DNN::isFlagSet (ModeOutputValues::DIRECT, layerData.outputMode ()));
                std::vector<double> probabilities = layerData.probabilities ();
                error = crossEntropy (begin (probabilities), end (probabilities), 
                                      begin (truth), end (truth), 
                                      layerData.deltasBegin (), layerData.deltasEnd (), 
                                      layerData.inverseActivationFunction (), 
                                      patternWeight);
                break;
            }
            case ModeErrorFunction::CROSSENTROPY_MUTUALEXCLUSIVE:
            {
                assert (!TMVA::DNN::isFlagSet (ModeOutputValues::DIRECT, layerData.outputMode ()));
                std::vector<double> probabilities = layerData.probabilities ();
                error = softMaxCrossEntropy (begin (probabilities), end (probabilities), 
                                             begin (truth), end (truth), 
                                             layerData.deltasBegin (), layerData.deltasEnd (), 
                                             layerData.inverseActivationFunction (), 
                                             patternWeight);
                break;
            }
            }
            if (factorWeightDecay != 0 && eRegularization != EnumRegularization::NONE)
            {
                error = weightDecay (error, itWeight, itWeightEnd, factorWeightDecay, eRegularization);
            }
            return error;
        } 








/*! \brief pre-training
 *
 * in development
 */
        template <typename Minimizer>
            void Net::preTrain (std::vector<double>& weights,
                                std::vector<Pattern>& trainPattern,
                                const std::vector<Pattern>& testPattern,
                                Minimizer& minimizer, Settings& settings)
        {
            auto itWeightGeneral = std::begin (weights);
            std::vector<Pattern> prePatternTrain (trainPattern.size ());
            std::vector<Pattern> prePatternTest (testPattern.size ());

            size_t _inputSize = inputSize ();

            // transform pattern using the created preNet
            auto initializePrePattern = [&](const std::vector<Pattern>& pttrnInput, std::vector<Pattern>& pttrnOutput)
                {
                    pttrnOutput.clear ();
                    std::transform (std::begin (pttrnInput), std::end (pttrnInput),
                                    std::back_inserter (pttrnOutput), 
                                    [](const Pattern& p)
            {
                Pattern pat (p.input (), p.input (), p.weight ());
                return pat;
            });
                };

            initializePrePattern (trainPattern, prePatternTrain);
            initializePrePattern (testPattern, prePatternTest);

            std::vector<double> originalDropFractions = settings.dropFractions ();

            for (auto& _layer : layers ())
            {
                // compute number of weights (as a function of the number of incoming nodes)
                // fetch number of nodes
                size_t numNodes = _layer.numNodes ();
                size_t numWeights = _layer.numWeights (_inputSize);

                // ------------------
                DNN::Net preNet;
                if (!originalDropFractions.empty ())
                {
                    originalDropFractions.erase (originalDropFractions.begin ());
                    settings.setDropOut (originalDropFractions.begin (), originalDropFractions.end (), settings.dropRepetitions ());
                }
                std::vector<double> preWeights;

                // define the preNet (pretraining-net) for this layer
                // outputSize == inputSize, because this is an autoencoder;
                preNet.setInputSize (_inputSize);
                preNet.addLayer (DNN::Layer (numNodes, _layer.activationFunctionType ()));
                preNet.addLayer (DNN::Layer (_inputSize, DNN::EnumFunction::LINEAR, DNN::ModeOutputValues::DIRECT)); 
                preNet.setErrorFunction (DNN::ModeErrorFunction::SUMOFSQUARES);
                preNet.setOutputSize (_inputSize); // outputSize is the inputSize (autoencoder)

                // initialize weights
                preNet.initializeWeights (DNN::WeightInitializationStrategy::XAVIERUNIFORM, 
                                          std::back_inserter (preWeights));

                // overwrite already existing weights from the "general" weights
                std::copy (itWeightGeneral, itWeightGeneral+numWeights, preWeights.begin ());
                std::copy (itWeightGeneral, itWeightGeneral+numWeights, preWeights.begin ()+numWeights); // set identical weights for the temporary output layer
            

                // train the "preNet"
                preNet.train (preWeights, prePatternTrain, prePatternTest, minimizer, settings);

                // fetch the pre-trained weights (without the output part of the autoencoder)
                std::copy (std::begin (preWeights), std::begin (preWeights) + numWeights, itWeightGeneral);

                // advance the iterator on the incoming weights
                itWeightGeneral += numWeights;

                // remove the weights of the output layer of the preNet
                preWeights.erase (preWeights.begin () + numWeights, preWeights.end ());

                // remove the outputLayer of the preNet
                preNet.removeLayer ();

                // set the output size to the number of nodes in the new output layer (== last hidden layer)
                preNet.setOutputSize (numNodes);
            
                // transform pattern using the created preNet
                auto proceedPattern = [&](std::vector<Pattern>& pttrn)
                    {
                        std::vector<Pattern> newPttrn;
                        std::for_each (std::begin (pttrn), std::end (pttrn),
                                       [&preNet,&preWeights,&newPttrn](Pattern& p)
                {
                    std::vector<double> output = preNet.compute (p.input (), preWeights);
                    Pattern pat (output, output, p.weight ());
                    newPttrn.push_back (pat);
//                    p = pat;
                });
                        return newPttrn;
                    };


                prePatternTrain = proceedPattern (prePatternTrain);
                prePatternTest = proceedPattern (prePatternTest);


                // the new input size is the output size of the already reduced preNet
                _inputSize = preNet.layers ().back ().numNodes ();
            }
        }
















    }; // namespace DNN
}; // namespace TMVA

#endif