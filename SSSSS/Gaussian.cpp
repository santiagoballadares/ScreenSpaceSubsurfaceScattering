#include "Gaussian.h"

// Class to initialize the vectors for the Skin Gaussian
class SkinGaussianSum : public std::vector<Gaussian>
{
    public:
        SkinGaussianSum()
		{
			// We use the unblurred image as an aproximation to the first 
            // gaussian because it is too narrow to be noticeable. The weight
            // of the unblurred image is the first one.
			float variances[] = { 0.0516500425655f, 0.271928080903f, 2.00626388153f };
			Vector3 weights[] = {
									Vector3(0.240516183695f, 0.447403391891f, 0.615796108321f), 
									Vector3(0.115857499765f, 0.366176401412f, 0.343917471552f),
									Vector3(0.183619017698f, 0.186420206697f, 0.0f),
									Vector3(0.460007298842f, 0.0f, 0.0402864201267f)
								};

			vector<Gaussian> &gaussianSum = *this;
			gaussianSum = Gaussian::gaussianSum(variances, weights, 3);
		}
};

class MarbleGaussianSum : public std::vector<Gaussian>
{
    public:
        MarbleGaussianSum()
		{
            // In this case the first gaussian is wide and thus we cannot
            // approximate it with the unblurred image. For this reason the
            // first weight is set to zero.
			float variances[] = { 0.0362208693441f, 0.114450574559f, 0.455584392509f, 3.48331959682f };
            Vector3 weights[] = {
									Vector3(0.0f, 0.0f, 0.0f),
									Vector3(0.0544578254963f, 0.12454890956f, 0.217724878147f),
									Vector3(0.243663230592f, 0.243532369381f, 0.18904245481f),
									Vector3(0.310530428621f, 0.315816663292f, 0.374244725886f),
									Vector3(0.391348515291f, 0.316102057768f, 0.218987941157f)
								};
            
            vector<Gaussian> &gaussianSum = *this;
            gaussianSum = Gaussian::gaussianSum(variances, weights, 4);
        }
};

const vector<Gaussian> Gaussian::SKIN = SkinGaussianSum();
const vector<Gaussian> Gaussian::MARBLE = MarbleGaussianSum();

vector<Gaussian> Gaussian::gaussianSum(float variances[], Vector3 weights[], int numVariances)
{
    vector<Gaussian> gaussians;

    for (int i = 0; i < numVariances; ++i)
	{
		float variance = 0;
		
		if (i == 0)
		{
			variance = variances[i];
		}
		else
		{
			variance = variances[i] - variances[i - 1];
		}

        gaussians.push_back(Gaussian(variance, weights, i));
    }

    return gaussians;
}


Gaussian::Gaussian(float variance, Vector3 weights[], int n) : width(sqrt(variance))
{
    Vector3 total = Vector3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < n + 2; ++i)
	{
        total += weights[i];
    }

    weight = Vector4(weights[n + 1].x, weights[n + 1].y, weights[n + 1].z, 1.0f);
	weight.x *= 1.0f / total.x;
	weight.y *= 1.0f / total.y;
	weight.z *= 1.0f / total.z;
}