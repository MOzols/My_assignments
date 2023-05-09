#include<vector>
#include<iostream>
using namespace std;

#include <glut.h>
#include <math.h>
#include <algorithm>
#include <stdlib.h>

#include "Vec3.h"
#include "Image.h"
#include "Ray.h"
#include "Sphere.h"

#define PI 3.1415926535



class Camera {
private:
	Vec3f rotation;

public:
	Vec3f position;
	float viewportWidth;
	float viewportHeight;

	Camera(Vec3f rotation, Vec3f position, float viewportWidth, float viewportHeight) {
		this->rotation.x = rotation.x * PI / 180.0;
		this->rotation.y = rotation.y * PI / 180.0;
		this->rotation.z = rotation.z * PI / 180.0;
		this->position = position;
		this->viewportWidth = viewportWidth;
		this->viewportHeight = viewportHeight;
	}

	void rotateRay(Vec3f &vec) {
		float x, y, z;

		//Rotate around x-axis
		y = vec.y * cos(rotation.x) - vec.z * sin(rotation.x);
		z = vec.y * sin(rotation.x) + vec.z * cos(rotation.x);
		vec.y = y; vec.z = z;

		//Rotate around y-axis
		x = vec.x * cos(rotation.y) + vec.z * sin(rotation.y);
		z = -(vec.x) * sin(rotation.y) + vec.z * cos(rotation.y);
		vec.x = x; vec.z = z;

		//Rotate around z-axis
		x = vec.x * cos(rotation.z) - vec.y * sin(rotation.z);
		y = vec.x * sin(rotation.z) + vec.y * cos(rotation.z);
		vec.x = x; vec.y = y;
	}
};

class Scene {
public:
	vector<Sphere> spheres;		
	vector<LightPoint> lights;

	Scene(void) {
		 
	}
	void add(Sphere & s) {
		spheres.push_back(s); 
		//cout << "Sphere added: " << "r = " << spheres[spheres.size()-1].r << endl;
	}
	void add(LightPoint & p) {
		lights.push_back(p);
	}

	void load(char * fileName) {
		// load a file with spheres for your scene here ...
		// Note: You do not have to do this as part of the assignment.
		// This is for the sake of convenience, if you want to save and setup many interesting scenes
	}

};


void glSetPixel(int x, int y, Vec3f & c) {
	glColor3f(c.r, c.g, c.b);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}

class SimpleRayTracer {
private: 
	Camera * camera;
	Scene * scene;
	Image * image;
	int numSamples;

	Vec3f getEyeRayDirection(float x, float y) {
		//Uses a fix camera looking along the negative z-axis
		static float z = -5.0f;		
		static float sizeX = camera->viewportWidth; 
		static float sizeY = camera->viewportHeight;
		static float left = -sizeX * 0.5f;
		static float bottom = -sizeY * 0.5f;
		static float dx =  sizeX / float(image->getWidth());  
		static float dy =  sizeY / float(image->getHeight());
	
		Vec3f ray = Vec3f(left + x * dx, bottom + y * dy, z);
		camera->rotateRay(ray);
		return ray.normalize();
	}


public:
	SimpleRayTracer(Scene * scene, Image * image, Camera * camera, int numSamples) {
		this->camera = camera;
		this->scene = scene;
		this->image = image;
		this->numSamples = numSamples;
	}

	bool searchAny(const Ray& ray) {
		for (int i = 0; i < scene->spheres.size(); i++) {
			HitRec hitRecCheck;
			if (scene->spheres[i].hit(ray, hitRecCheck)) {
				return true;
			}
		}

		return false;
	}

	void searchClosestHit(const Ray & ray, HitRec & hitRec) {
		for (int i = 0; i < scene->spheres.size(); i++) {
			HitRec hitRecCheck;
			hitRecCheck.primIndex = i;
			bool hit = scene->spheres[i].hit(ray, hitRecCheck);
			
			if (hit && hitRecCheck.tHit < hitRec.tHit) {
				hitRec = hitRecCheck;
			}
		}
	}

	Vec3f fireSingleRay(Ray r, int recursions) {
		if (recursions > 3) {
			return Vec3f(0.0f, 0.0f, 1.0f); //Blue bg
		}

		HitRec hitRec;
		hitRec.tHit = INFINITY;
		hitRec.anyHit = false;

		searchClosestHit(r, hitRec);

		if (!(hitRec.anyHit)) {
			return Vec3f(0.0f, 0.0f, 1.0f); //Blue bg
		}

		Sphere& s = scene->spheres[hitRec.primIndex];
		s.computeSurfaceHitFields(r, hitRec);

		Vec3f color = Vec3f(0, 0, 0);

		for (size_t l = 0; l < scene->lights.size(); l++) {
			LightPoint& p = scene->lights[l];

			Vec3f ambTerm = p.ambient * s.mat.ambient;
			color += ambTerm;

			Vec3f lightDir = (p.position - hitRec.p).normalize();
			//if shadow, continue
			Ray shadowRay(hitRec.p + hitRec.n * 0.1, lightDir); //todo: add a limit to ignore spheres behind light source (just calculate distance to light source?)
			if (searchAny(shadowRay)) continue;

			float diff = max(hitRec.n.dot(lightDir), 0.0f);

			Vec3f diffTerm = p.diffuse * s.mat.diffuse * diff;

			Vec3f viewDir = -(r.d);
			Vec3f reflectDir = (-lightDir) - hitRec.n * 2.0f * (-lightDir).dot(hitRec.n);
			float spec = pow(max(viewDir.dot(reflectDir), 0.0f), s.mat.shininess);

			Vec3f specTerm = p.specular * s.mat.specular * spec;

			color += diffTerm + specTerm;
		}

		color.r = min(color.r, 1.0f);
		color.g = min(color.g, 1.0f);
		color.b = min(color.b, 1.0f);
		color *= 1 - s.mat.reflectiveness;

		Vec3f reflectionDir = r.d - hitRec.n * 2.0f * hitRec.n.dot(r.d);
		Ray reflection(hitRec.p + hitRec.n * 0.1, reflectionDir);
		color += fireSingleRay(reflection, recursions + 1) * s.mat.reflectiveness;

		return color;
	}

	void fireRays(void) { 
		Ray ray;
		//bool hit = false;
		ray.o = camera->position; //Set the start position of the eye rays to the camera's position

		for (int y = 0; y < image->getHeight(); y++) {
			for (int x = 0; x < image->getWidth(); x++) {
				Vec3f c = Vec3f(0, 0, 0);

				for (int s = 0; s < this->numSamples; s++) {
					float offX = (float) rand() / RAND_MAX;
					float offY = (float) rand() / RAND_MAX;

					ray.d = getEyeRayDirection(x+offX, y+offY);
					c += fireSingleRay(ray, 0);
				}

				if (this->numSamples == 0) {
					ray.d = getEyeRayDirection(x, y);
					c += fireSingleRay(ray, 0);
				}
				else {
					c *= 1.0f / this->numSamples;
				}

				image->setPixel(x, y, c);
				glSetPixel(x, y, c);
			}
		}
	}
};


SimpleRayTracer * rayTracer;

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//kolla vad klockan är nu
	rayTracer->fireRays();
	//kolla vad klockan är igen

	glFlush();
}

void changeSize(int w, int h) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);
	glViewport(0,0,w,h);
}

void init(void)
{
	
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(640, 480);
	glutCreateWindow("SimpleRayTracer");
	glutDisplayFunc(display);
	glutReshapeFunc(changeSize);
	//glutKeyboardFunc(keypress);

	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);

	Scene * scene = new Scene;	

	LightPoint light = { Vec3f(5, 15, 15), Vec3f(0.3, 0.3, 0.3), Vec3f(1, 1, 1), Vec3f(1, 1, 1) };
	scene->add(light);

	Material m1 = { Vec3f(0.192250, 0.192250, 0.192250), Vec3f(0.507540, 0.507540, 0.507540), Vec3f(0.508273, 0.508273, 0.508273), 51.2, 0.8 };
	Sphere s1(Vec3f(0.0, 0.0, -400.0), 380, m1);
	scene->add(s1);

	Material m2 = { Vec3f(0.17450, 0.011750, 0.011750), Vec3f(0.614240, 0.041360, 0.041360), Vec3f(0.727811, 0.626959, 0.626959), 76.8, 0 };
	Sphere s2(Vec3f(0.0, 0.0, 0.0), 5, m2);
	scene->add(s2);

	Image * image = new Image(640, 480);
	
	Camera * c = new Camera(Vec3f(0, -30, 0), Vec3f(-20, 0, 60), 4.0, 3.0);

	int numSamples;
	std::cout << "Enter number of samples: ";
	std::cin >> numSamples;

	rayTracer = new SimpleRayTracer(scene, image, c, numSamples);

}

void main(int argc, char **argv) {
	glutInit(&argc, argv);
	init();
	glutMainLoop();
}
