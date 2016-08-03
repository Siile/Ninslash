// Inline file, no header guard!
// Included by render.h


static double CubicRoot(double x)
{
	if(x == 0.0)
		return 0.0;
	else if(x < 0.0)
		return -exp(log(-x)/3.0);
	else
		return exp(log(x)/3.0);
}

static float SolveBezier(float x, float p0, float p1, float p2, float p3)
{
	// check for valid f-curve
	// we only take care of monotonic bezier curves, so there has to be exactly 1 real solution
	tl_assert(p0 <= x && x <= p3);
	tl_assert((p0 <= p1 && p1 <= p3) && (p0 <= p2 && p2 <= p3));

	double a, b, c, t;
	double x3 = -p0 + 3*p1 - 3*p2 + p3;
	double x2 = 3*p0 - 6*p1 + 3*p2;
	double x1 = -3*p0 + 3*p1;
	double x0 = p0 - x;

	if(x3 == 0.0 && x2 == 0.0)
	{
		// linear
		// a*t + b = 0
		a = x1;
		b = x0;

		if(a == 0.0)
			return 0.0f;
		else
			return -b/a;
	}
	else if(x3 == 0.0)
	{
		// quadratic
		// t*t + b*t +c = 0
		b = x1/x2;
		c = x0/x2;

		if(c == 0.0)
			return 0.0f;

		double D = b*b - 4*c;

		t = (-b + sqrt(D))/2;

		if(0.0 <= t && t <= 1.0001f)
			return t;
		else
			return (-b - sqrt(D))/2;
	}
	else
	{
		// cubic
		// t*t*t + a*t*t + b*t*t + c = 0
		a = x2 / x3;
		b = x1 / x3;
		c = x0 / x3;

		// substitute t = y - a/3
		double sub = a/3.0;

		// depressed form x^3 + px + q = 0
		// cardano's method
		double p = b/3 - a*a/9;
		double q = (2*a*a*a/27 - a*b/3 + c)/2;
		
		double D = q*q + p*p*p;

		if(D > 0.0)
		{
			// only one 'real' solution
			double s = sqrt(D);
			return CubicRoot(s-q) - CubicRoot(s+q) - sub;
		}
		else if(D == 0.0)
		{
			// one single, one double solution or triple solution
			double s = CubicRoot(-q);
			t = 2*s - sub;
			
			if(0.0 <= t && t <= 1.0001f)
				return t;
			else
				return (-s - sub);

		}
		else
		{
			// Casus irreductibilis ... ,_,
			double phi = acos(-q / sqrt(-(p*p*p))) / 3;
			double s = 2*sqrt(-p);

			t = s*cos(phi) - sub;

			if(0.0 <= t && t <= 1.0001f)
				return t;

			t = -s*cos(phi+pi/3) - sub;

			if(0.0 <= t && t <= 1.0001f)
				return t;
			else
				return -s*cos(phi-pi/3) - sub;
		}
	}
}

template<typename T, typename TB>
T bezier(T p0, T p1, T p2, T p3, TB a, T (*lerp)(T a, T b, TB amount))
{
	// De-Casteljau Algorithm
	const T c10 = lerp(p0, p1, a);
	const T c11 = lerp(p1, p2, a);
	const T c12 = lerp(p2, p3, a);

	const T c20 = lerp(c10, c11, a);
	const T c21 = lerp(c11, c12, a);

	return lerp(c20, c21, a); // c30
}

template<typename TKeyframe>
void CRenderTools::RenderEvalSkeletonAnim(TKeyframe *pKeyFrame, int NumKeyframes, float Time, typename TKeyframe::KeyframeReturnType *pResult)
{
	typedef typename TKeyframe::KeyframeReturnType ResultType;

	const int NumResults = TKeyframe::NumValues;
	dbg_assert(NumResults <= 2, "Invalid number of result values");

	if(NumKeyframes == 0)
	{
		for(int i = 0; i < NumResults; i++)
			pResult[i] = ResultType(TKeyframe::GetValueDefault());

		return;
	}

	if(NumKeyframes == 1)
	{
		switch(NumResults)
		{
		// fall through
		case 2:	pResult[1] = pKeyFrame[0].GetValue1();
		case 1: pResult[0] = pKeyFrame[0].GetValue0();
		}

		return;
	}

	Time = fmod(Time, pKeyFrame[NumKeyframes-1].m_Time);
	for(int i = 0; i < NumKeyframes-1; i++)
	{
		if(Time >= pKeyFrame[i].m_Time && Time <= pKeyFrame[i+1].m_Time)
		{
			float Delta = pKeyFrame[i+1].m_Time-pKeyFrame[i].m_Time;
			float a = (Time-pKeyFrame[i].m_Time)/Delta;

			int CurveType = pKeyFrame[i].GetCurveType();

			if(CurveType == SPINE_CURVE_LINEAR)
			{
				switch(NumResults)
				{
				// fall through
				case 2:	pResult[1] = TKeyframe::interpolate(pKeyFrame[i].GetValue1(), pKeyFrame[i+1].GetValue1(), a);
				case 1: pResult[0] = TKeyframe::interpolate(pKeyFrame[i].GetValue0(), pKeyFrame[i+1].GetValue0(), a);
				}
			}
			else if(CurveType == SPINE_CURVE_STEPPED)
			{
				switch(NumResults)
				{
				// fall through
				case 2:	pResult[1] = pKeyFrame[i].GetValue1();
				case 1: pResult[0] = pKeyFrame[i].GetValue0();
				}
			}
			else if(CurveType == SPINE_CURVE_BEZIER)
			{
				float param = clamp(SolveBezier(a, 0.0f, pKeyFrame[i].m_Curve.m_lPoints[0], pKeyFrame[i].m_Curve.m_lPoints[2], 1.0f), 0.0f, 1.0f);
				float t = bezier(0.0f, pKeyFrame[i].m_Curve.m_lPoints[1], pKeyFrame[i].m_Curve.m_lPoints[3], 1.0f, param, TKeyframe::interpolate);
				switch(NumResults)
				{
				// fall through
				case 2: pResult[1] = TKeyframe::interpolate(pKeyFrame[i].GetValue1(), pKeyFrame[i+1].GetValue1(), t);
				case 1: pResult[0] = TKeyframe::interpolate(pKeyFrame[i].GetValue0(), pKeyFrame[i+1].GetValue0(), t);
				}
			}

			return;
		}
	}

	// last point
	switch(NumResults)
	{
	// fall through
	case 2:	pResult[1] = pKeyFrame[NumKeyframes-1].GetValue1();
	case 1: pResult[0] = pKeyFrame[NumKeyframes-1].GetValue0();
	}
}