#pragma once

/*
 * Class that encapsulates a function to build a Gaussian from variances and weights.
 */
#include <vector>
#include "../Framework/Vector3.h"
#include "../Framework/Vector4.h"

using namespace std;

class Gaussian
{
    public:
        static std::vector<Gaussian> gaussianSum(float variances[], Vector3 weights[], int numVariances);

        float	getWidth()	const { return width; }
        Vector4	getWeight()	const { return weight; }

        static const std::vector<Gaussian> SKIN;
		static const std::vector<Gaussian> MARBLE;

    private:
        Gaussian() {} 
        Gaussian(float variance, Vector3 weights[], int n);

        float width;
		Vector4 weight;
};
