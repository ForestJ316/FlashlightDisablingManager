#pragma once
// https://www.pcg-random.org/posts/ease-of-use-without-loss-of-power.html
#include "randutils.hpp"

namespace Utils
{
	static randutils::default_rng rng;

	inline float GetRandomFloat(float a_min, float a_max)
	{
		float uni_dist = rng.uniform(a_min, a_max);
		return uni_dist;
	}

	inline int GetRandomInt(int a_min, int a_max)
	{
		int uni_dist = rng.uniform(a_min, a_max);
		return uni_dist;
	}
}
