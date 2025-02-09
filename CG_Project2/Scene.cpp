// Scene.cpp: implementation of the CScene class.
//
//////////////////////////////////////////////////////////////////////

#include "Scene.h"
 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScene::CScene()
{

	// initialize scene transformation
	tx = 0;
	ty = 0.0;
	tz = 0;
	rx = 0.0;
	ry = 0.0;
	rz = 0.0;
	scale = 1.0;


	// initialize clock
	clock = 0;

	// initialize frameNumber
	frameNumber = 0;

	// initialize pointers
	pMainBug= NULL;
	
	for (int i=0; i<numCritters; i++){
		pCritters[i] = NULL;
	}

	for (int i=0; i<numObstacles; i++){
		pObstacles[i] = NULL;
	}

	build();

}

CScene::~CScene()
{

	delete pMainBug;

	for (int i=1; i<numCritters; i++){
		delete pCritters[i];
	}	
	
	for (int i=1; i<numObstacles; i++){
		delete pObstacles[i];
	}
}

void CScene::build()
{

	// Adds trees as obstacles
	for (int i=0; i<numObstacles; i++)
	{

		pObstacles[i]=  new CTree(rgen, 5, 4, 4.5f, 0.6f, 0, (rgen.randu()-0.5)*2.0+3.0);
		
		
	}


	// ... you can add in code here to create more obstacles

	// create a main bug
	Point3d loc = freeSpotUniform(1.0f, 1.5f);
	pMainBug = new CBug(1.0f, loc.x, loc.y, (rgen.randg())*0.1f, (rgen.randg())*0.1f);

	// create follower critters
	for (int index = 0; index < numCritters; index++)
	{
		Point3d loc = freeSpotUniform(1.0f, 1.5f);
		pCritters[index] = new CBug(0.6f, loc.x, loc.y, (rgen.randg())*0.1f, (rgen.randg())*0.1f);
	}
	
	
	// reset clock
	resetClock();

}

Point3d CScene::freeSpotUniform(float minDist, float rad)
{
	Point3d p;
	do{
		p.set(2*rgen.randu()*rad - rad, 2*rgen.randu()*rad - rad, 0);
	}while (!isFreeSpot(p, minDist));
	return p;
}

Point3d CScene::freeSpotGaussian(float minDist, float sigma)
{
	Point3d p;
	do{
		p.set((rgen.randg())*sigma, (rgen.randg())*sigma, 0);
	}while (!isFreeSpot(p, minDist));
	return p;
}

bool CScene::isFreeSpot(Point3d p, float minDist)
{
	// check obstacles if any
	for (int i=0; i<numObstacles; i++){
		if (p.distance((pObstacles[i])->getLocation())<minDist)
			return false;
	}

	// check main bug if available
	if(pMainBug!=NULL){

		if(p.distance(pMainBug->getLocation()) < minDist)
			return false;
	}
	
	// take into account follower critters when determining a free spot.
	// check follower critters

	for (int index = 0; index < numCritters; index++)
	{
		if (pCritters[index] != NULL) {
			if (p.distance( (pCritters[index])->getLocation() ) < minDist) 
				return false;
		}
	}


	return true;
}

void CScene::sceneTransformation()
{
	
	glTranslated(tx, ty, tz);
	glRotated(rz, 0, 0, 1);
	glRotated(ry, 0, 1, 0);
	glRotated(rx, 1, 0, 0);
	glScaled(scale, scale, scale);
}



void CScene::draw(bool shadowFlag)
{

	int i;
	double ground[] = {0.0, 0.0, 1.0, 0.001};

	// draw ground plane (a square at Z=0)
	glColor3d(0.4, 0.8, 0.35);
	glBegin(GL_POLYGON);
	glNormal3d(0, 0, 1);
	glVertex3d(15, -15, 0);
	glVertex3d(15, 15, 0);
	glVertex3d(-15, 15, 0);
	glVertex3d(-15, -15, 0);
	glEnd();


	// draw critters and bugs
	Vector3d color(0.9, 0.1, 0.1);
	pMainBug->draw(true, color);

	// ... add in code here
	// draw follower bugs
	
	for (int index = 0; index < numCritters ; index++)
		pCritters[index]->draw(true,color);

	// enable clipping plane before drawing obstacles
	glClipPlane(GL_CLIP_PLANE0, ground);
	glEnable(GL_CLIP_PLANE0);

	// draw obstacles
	for (i=0; i<numObstacles; i++){
		pObstacles[i]->draw(true);
	}

	// to draw shadow for obstacles, bugs and other scene elements
	// shadowProject() computes the projection matrix and is already provided.
	if (shadowFlag)
	{
		shadowProject();
		glDisable(GL_LIGHT0);
		glPushMatrix();
			//Main bug shadow:
			pMainBug->draw(true, color);

			//Shadows for other bugs: 
			for (int index = 0; index < numCritters; index++)
				pCritters[index]->draw(true,color);

			//Obstracles' shadows:
			for (i=0; i<numObstacles; i++)
			{
				pObstacles[i]->draw(true);
			}
		glPopMatrix();
		glEnable(GL_LIGHT0);
	}
	
}

void CScene::process(double elapsed)
{
	// elapsed: time elapsed since last frame (last display)

	// update clock
	clock += elapsed;
	frameNumber++;

	double dtMax = 1.0/50.0f;  // maximum time needed to make one step forward

	//Values:
	double kRepulsion = 4;
	double kAttraction = -2;

	double expAttraction = 2;
	double expRepulsion = -0.8;
	
	double objectRepulsion = -0.5;

	// number of integration steps
	int steps = (int) (elapsed/dtMax);

	if(steps<1) steps =1;


	for (int st=0; st <steps; st++){
		double dt = elapsed/((double) steps);		// time spent per step
		
		// initialize acceleration
		pMainBug->accelReset();


		// ... add in code here to reset all follower critters' acceleration
		for (int index = 0; index < numCritters; index++)
			pCritters[index]->accelReset();
		
 		// take into account all sorts of forces to update acceleration


		// ... All critters do not hit obstacles, if any
		for (int index = 0; index < numObstacles; ++index)
		{
			//Main bug tries not to hit obstacles:
			pMainBug->accelAttract(pObstacles[index]->getLocation(),kRepulsion, objectRepulsion);

			//Follower bugs try not to hit obstacles:
			for (int critterIndex = 0; critterIndex < numCritters; ++critterIndex)
				pCritters[critterIndex]->accelAttract(pObstacles[index]->getLocation(), kRepulsion , objectRepulsion);
		}
		
		// ... follower critters follow the main bug
		for (int index = 0; index < numCritters; ++ index)
			pCritters[index]->accelAttract(pMainBug->getLocation(), kAttraction, expAttraction);
		

		// ... follower critters try not to hit each other
		for (int index1 = 0; index1 < numCritters; ++ index1)
			for (int index2 = 0; index2 < numCritters; ++ index2)
				if (index1 != index2) //if not same bug
				{
					pCritters[index1]->accelAttract(pCritters[index2]->getLocation(), kRepulsion, expRepulsion);
				}
		
		// ... follower critters try not to hit the main bug
		for (int index1 = 0; index1 < numCritters; ++ index1)
			pCritters[index1]->accelAttract(pMainBug->getLocation(), kRepulsion , expRepulsion);
		
		// apply other accelerations
		// main bug wanders every once in a while, but stays near the middle
		pMainBug->accelWander(-1.2, clock, ((rgen.randg()+0.5)*3.0+4.0));

		// .... viscous drag for main bug and follower critters

		//Main bug:
		pMainBug->accelDrag(2);

		//Follower bugs:
		for (int index = 0; index < numCritters; ++index)
			pCritters[index]->accelDrag(2);
		
		// ...  cap on main bug and follower critter accelerations; accelCap() function is given in CCritter.h
		
		//Main bug:
		pMainBug->accelCap(4);

		//Other bugs:
		for (int index = 0; index < numCritters; ++index)
			pCritters[index]->accelCap(5);

		// ... integrate to update velocity and position for main bug and all follower critters

		//Main Bug:
		pMainBug->integrate(dt);

		//Followers:
		for (int index = 0; index < numCritters; ++index)
			pCritters[index]->integrate(dt);

	}

	// ... key frame motion to update keyframe parameters for main bug and followers; call keyframe() in CBug class

	//Main Bug:
	pMainBug->keyframe(pMainBug->distTraveled());

	//Followers:
	for (int index = 0; index < numCritters; ++index)
			pCritters[index]->keyframe(pCritters[index]->distTraveled());
	
}

void CScene::shadowProject()
{
	
	// shadow matrix for drop shadow onto z=0 plane
	float *m = new float[4*4];

	float dx = light_position[0];
	float dy = light_position[1];
	float dz = light_position[2];

	float a=0, b=0, c=1, d = -0.01f;

	m[0*4+0]= b*dy + c*dz;
	m[1*4+0]= -b*dx;
	m[2*4+0]= -c*dx;
	m[3*4+0]= -d*dx;

	m[0*4+1]= -a*dy;
	m[1*4+1]= a*dx + c*dz;
	m[2*4+1]= -c*dy;
	m[3*4+1]= -d*dy;

	m[0*4+2]= -a*dz;
	m[1*4+2]= -b*dz;
	m[2*4+2]= a*dx+b*dy;
	m[3*4+2]= -d*dz;

	m[0*4+3]=  0;
	m[1*4+3]=  0;
	m[2*4+3]=  0;
	m[3*4+3]=  a*dx + b*dy + c*dz;

	glMultMatrixf((const float *) m);
	
	delete [] m;
}
