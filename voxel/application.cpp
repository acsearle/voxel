//
//  application.cpp
//  voxel
//
//  Created by Antony Searle on 2/10/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#include <cstdlib>
#include <fstream>
#include <iostream>

#include <OpenGL/gl3.h>
#include "btBulletDynamicsCommon.h"

#include "application.h"
#include "shader.h"
#include "program.h"
#include "utility.h"
#include "mat.h"
#include "vec.h"
#include "mesh.h"
#include "vertex.h"
#include "voxel.h"
#include "Texture.h"

using namespace std;

application::~application() {
}



class MyVertex {
public:
    vec<GLubyte, 3> inPosition;
    vec<GLubyte, 2> inTexcoord;
    static VertexDescription describe();
};

VertexDescription MyVertex::describe() {
    VertexDescription d;
    d.stride = sizeof(MyVertex);
    d.attribs[0] = unique_ptr<VertexAttributeDescription>{new VertexAttributeDescription{
        3, GL_UNSIGNED_BYTE, GL_FALSE, offsetof(MyVertex, inPosition)}};
    d.attribs[8] = unique_ptr<VertexAttributeDescription>{new VertexAttributeDescription{
        2, GL_UNSIGNED_BYTE, GL_FALSE, offsetof(MyVertex, inTexcoord)}};
    return move(d);
}




// we can get strided and typed pointers to thngs as required

// my_vertex becomes a convenience class when dealing with known data


class buffer : public named {
public:
    buffer() { glGenBuffers(1, &name_); }
    ~buffer() { glDeleteBuffers(1, &name_); }
    buffer& bind(GLenum target) { glBindBuffer(target, name_); return *this;}
    template<typename T> buffer& data(GLenum target, vector<T> data, GLenum usage) {
        glBufferData(target, sizeof(T) * data.size(), data.data(), usage);
        return *this;
    }
};


class vertex_array : public named {
public:
    vertex_array() { glGenVertexArrays(1, &name_); }
    ~vertex_array() { glDeleteVertexArrays(1, &name_); }
    vertex_array& bind() { glBindVertexArray(name_); return *this;}
    
};




class my_application : public application {
    
public:
    
    my_application();
    
    virtual ~my_application();
    
    virtual void render(size_t width, size_t height, double time);
    
private:
    
    unique_ptr<program> p;
    unique_ptr<Texture> textureColor;
    unique_ptr<Texture> textureNormal;
    buffer buf;
    buffer elm;
    vertex_array vao;
    GLsizei count;
    
    
    unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
	unique_ptr<btCollisionDispatcher> dispatcher;
	unique_ptr<btBroadphaseInterface> overlappingPairCache;
	unique_ptr<btSequentialImpulseConstraintSolver> solver;
	unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
    
};


unique_ptr<application> application::make() {
    return unique_ptr<application>{new my_application{}};
}




my_application::my_application() {
    
    cout << glGetString(GL_RENDERER) << endl;
    cout << glGetString(GL_VERSION) << endl;
    
    
    p = make_program("basic");
    
    opengl_health();
    
    glActiveTexture(GL_TEXTURE0);
    textureColor = makeTexture(*makeImageTextures());
    glActiveTexture(GL_TEXTURE1);
    textureNormal = makeTexture(*makeImageNormals());
    
    opengl_health();
    
    Voxel v{vec<size_t,3>{1,1,1}};
    v(0,0,0) = 1;
    auto m = v.makeMesh();
    buf.bind(GL_ARRAY_BUFFER).data(GL_ARRAY_BUFFER, m->vertices, GL_STATIC_DRAW);
    vao.bind();
    VoxelVertex::describe().use();
    p->validate();
    p->use();
    elm.bind(GL_ELEMENT_ARRAY_BUFFER).data(GL_ELEMENT_ARRAY_BUFFER, m->elements, GL_STATIC_DRAW);
    count = (GLsizei) m->elements.size();
    glEnable(GL_CULL_FACE);
    opengl_health();

    (*p)["textureMatrix"] = identity<float, 4>() * (1.0f/16.f);
    (*p)["samplerColor"] = 0;
    (*p)["samplerNormal"] = 1;
    glEnable(GL_DEPTH_TEST);
    opengl_health();
    
    // OpenGL init end
    
    // Bullet init begin
    
    
	collisionConfiguration.reset(new btDefaultCollisionConfiguration());
	dispatcher.reset(new btCollisionDispatcher(collisionConfiguration.get()));
	overlappingPairCache.reset(new btDbvtBroadphase());
	solver.reset(new btSequentialImpulseConstraintSolver);
	dynamicsWorld.reset(new btDiscreteDynamicsWorld(dispatcher.get(),
                                                    overlappingPairCache.get(),
                                                    solver.get(),
                                                    collisionConfiguration.get()));
    
	dynamicsWorld->setGravity(btVector3(0,-10,0));

    // Bullet init end
    
    // Ground
	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(0.5),btScalar(0.5),btScalar(0.5)));
    
	btAlignedObjectArray<btCollisionShape*> collisionShapes;
    
	collisionShapes.push_back(groundShape);
    
	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0,0,0));
    
	{
		btScalar mass(0.);
        
		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);
        
		btVector3 localInertia(0,0,0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass,localInertia);
        
		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
        
		//add the body to the dynamics world
		dynamicsWorld->addRigidBody(body);
	}
    
    {
		//create a dynamic rigidbody
        
		btCollisionShape* colShape = new btBoxShape(btVector3(0.5,0.5,0.5));
		//btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);
        
		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();
        
		btScalar	mass(1.f);
        
		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);
        
		btVector3 localInertia(0,0,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);
        
        startTransform.setOrigin(btVector3(0.5,2,0));
		
        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        
        dynamicsWorld->addRigidBody(body);
	}

    {
		//create a dynamic rigidbody
        
		btCollisionShape* colShape = new btBoxShape(btVector3(0.5,0.5,0.5));
		//btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);
        
		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();
        
		btScalar	mass(1.f);
        
		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);
        
		btVector3 localInertia(0,0,0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass,localInertia);
        
        startTransform.setOrigin(btVector3(0.0,4,0));
		
        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        
        dynamicsWorld->addRigidBody(body);
	}

    
    
    
    
   
    
    
    
    
    
}

my_application::~my_application() {
}

void my_application::render(size_t width, size_t height, double time) {
    
    glViewport(0, 0, (int) width, (int) height);
    //glClearColor(rand()&1,rand()&1,rand()&1,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    (*p)["cameraProjectionMatrix"] = perspective((float) M_PI_4, width / (float) height, 1.f, 100.f);
    (*p)["cameraViewMatrix"] = lookat(vec<float, 3>(4.0f, 8.0f, 12.0f),
                                      vec<float, 3>(0.0f, 0.0f, 0.0f),
                                      vec<float, 3>(0.0f, 1.0f, 0.0f));

    
    dynamicsWorld->stepSimulation(1.f/60.f,10);
    
    //print positions of all objects
    for (int j=dynamicsWorld->getNumCollisionObjects()-1; j>=0 ;j--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            btTransform trans;
            body->getMotionState()->getWorldTransform(trans);
            printf("world pos = %f,%f,%f\n",float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()));
            
            
            
            //mat<float, 4, 4> model = rotateY((float) (0)) * translate(vec<float, 3>(-0.5f, -0.5f, -0.5f));
            mat<float, 4, 4> model;
            trans.getOpenGLMatrix(model.data());
            model = model * translate(vec<float, 3>(-0.5f, -0.5f, -0.5f));
            (*p)["modelMatrix"] = model;
            (*p)["inverseTransposeModelMatrix"] = inverse(transpose(model));
            
            
            
            opengl_health();
            
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
            
            opengl_health();
            
            
            
            
            
        }
    }

    
    
    
    
    
    
    

    

    
    
}