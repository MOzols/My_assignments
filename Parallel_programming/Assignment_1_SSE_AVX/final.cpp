#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include<immintrin.h>

// d-dimensional point representation
template <int d> struct point {
	float v[d];
};

// overloading the << operator for 'point'
template <int d> std::ostream& operator<< (std::ostream& os, const point<d> &p) {
   os << "(" << p.v[0];
	for(int i=1; i<d; ++i) os << ',' << p.v[i];
	os << ")";
   return os;
}

// d-dimensional ball representation
template <int d> struct ball {
	point<d> center;
	float radius;
};

// overloading the << operator for 'ball'
template <int d> std::ostream& operator<< (std::ostream& os, const ball<d> &b) {
   os << "center=" << b.center << " radius=" << b.radius;
   return os;
}

// return square of x
inline float sq(const float &x) {
	return x*x;
}

// return square distance between two d-dimentional points: a and b
template <int d> inline float sqdst(const point<d> &a, const point<d> &b) {
	float sum = 0.0f;
	for(int i=0; i<d; ++i)
		sum += sq(a.v[i]-b.v[i]);
	return sum;
}

// mesh of n d-dimensional points stored according to the SoA layout
template <const int d> struct mesh {
	//constructor
	mesh(const int n) : n(n) {
		assert(n>0 && n%16==0);
		for(int i = 0; i < d; i++)
    		axes[i] = (float*)aligned_alloc(32,sizeof(float)*n);
		assert(axes);
	}
	//destructor
	~mesh() {
	    for(int i = 0; i < d; i++)
    		free(axes[i]);
	}
	void set(const point<d> p, int i) {
		assert(i>=0 && i<n);
		for(int j = 0; j < d; j++)
		    *(axes[j]+i) = p.v[j];
	}
	ball<d> calc_ball() {
		ball<d> b;
		// calc ball.center
		point<d> _min;
		point<d> _max;
        __m256 min, max, tmp;

        for(int i = 0; i < d; i++){
            min =_mm256_load_ps(axes[i]);//load in one of dimensions in variable min
            max =_mm256_load_ps(axes[i]);//load same dimension in variable max
		    for(int j=8; j<n; j+=8){
    		    min =_mm256_min_ps(min,_mm256_load_ps(axes[i]+j));//go through dimension compare and keep minimal points
    		    max =_mm256_max_ps(max,_mm256_load_ps(axes[i]+j));//go through dimension compare and keep maximal points
    		}
    		//Find minimal point among the last 8 values in current dimension
    		tmp =_mm256_permute2f128_ps(min,min,_MM_SHUFFLE(0,0,0,1));
    		min =_mm256_min_ps(min,tmp);
    		tmp =_mm256_permute_ps(min,_MM_SHUFFLE(1,0,3,2));
    		min =_mm256_min_ps(min,tmp);
    		tmp =_mm256_permute_ps(min,_MM_SHUFFLE(1,0,3,1));
    		min =_mm256_min_ps(min,tmp);
    		_min.v[i] = min[0];
            //Find maximal point among the last 8 values in current dimension
    		tmp = _mm256_permute2f128_ps(max,max,_MM_SHUFFLE(0,0,0,1));
    		max =_mm256_max_ps(max,tmp);
    		tmp =_mm256_permute_ps(max,_MM_SHUFFLE(1,0,3,2));
    		max =_mm256_max_ps(max,tmp);
    		tmp =_mm256_permute_ps(max,_MM_SHUFFLE(1,0,3,1));
    		max =_mm256_max_ps(max,tmp);
    		_max.v[i] = max[0];
    	}
    	//calulate center from minimal and maximal points and save it in ball
		for(int i=0; i<d; ++i)
			b.center.v[i] = (_max.v[i]-_min.v[i])*0.5f+_min.v[i];

		// calc ball.radius
        __m256 //intrinsic variables for 8 floats to keep values for
        ctr = _mm256_setzero_ps(),//center
        sub = _mm256_setzero_ps(),//subtraction
        sqdst = _mm256_setzero_ps(),//squer distance
        sum = _mm256_setzero_ps(),//sum
        maxsqdst = _mm256_setzero_ps();//maximal square distance
        tmp = _mm256_setzero_ps();//temporary to keep data for dimensions

        for(int i = 0; i < n; i+=8){
            for(int j = 0; j < d; j++){
                //std::cout<<"\nj="<<j<<"; i="<<i;
                tmp = _mm256_load_ps(axes[j]+i);//keeps data for some dimension in given range
                ctr = _mm256_set1_ps(b.center.v[j]);//load in appropriate center value depending on dimension
                sub = _mm256_sub_ps(tmp,ctr);//keep value of subtraction between dimension points and center
                sqdst = _mm256_mul_ps(sub,sub);//square of subtraction
                sum = _mm256_add_ps(sum,sqdst);//sum up all dimension results in particular range (vertical addition)
            }
            maxsqdst = _mm256_max_ps(maxsqdst,sum);//compare maximal square distance with newly made sum keep maximal values
            sum = _mm256_setzero_ps();//reset sum value with zeros
        }
        //Find maximal square distance among last 8 values
    	tmp =_mm256_permute2f128_ps(maxsqdst,maxsqdst,_MM_SHUFFLE(0,0,0,1));
    	maxsqdst =_mm256_max_ps(maxsqdst,tmp);
    	maxsqdst =_mm256_permute_ps(maxsqdst,_MM_SHUFFLE(1,0,3,2));
    	maxsqdst =_mm256_max_ps(maxsqdst,tmp);
    	tmp =_mm256_permute_ps(maxsqdst,_MM_SHUFFLE(1,0,3,1));
    	maxsqdst =_mm256_max_ps(maxsqdst,tmp);
    	//get value from square root of maximal square distance, gives radius, save result in ball
    	b.radius = sqrtf(maxsqdst[0]);
		return b;//return ball
	}
private:
	const int n = 0;
	float* axes[d];
};

template <const int d> inline point<d> randompoint() {
	point<d> p;
	for(int j=0; j<d; ++j)
		p.v[j] = 2.0f*rand()/RAND_MAX;
	return p;
}

//D is a parameter arbitrarily set at compile time in the command line
#ifndef D
#define D 3 //default value (can be changed)
#endif

int main(int argc, char *argv[]) {
	srand(12345);
	const int n = atoi(argv[1]);
	mesh<D> m(n);
	for(int i=0; i<n; ++i)
		m.set(randompoint<D>(), i);
	clock_t t = clock();
	ball<D> b = m.calc_ball();
	t = clock()-t;
	std::cout << b << " elapsed time=" << 1000.0*t/CLOCKS_PER_SEC << "ms" << std::endl;
	return 0;
}
