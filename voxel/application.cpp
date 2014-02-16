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



class VoxelBody {
public:
    mat<float, 4, 4> ATTRIBUTE_ALIGNED128(model);
    buffer buf;
    buffer elm;
    vertex_array vao;
    GLsizei count;
    unique_ptr<btRigidBody> body;
    
    VoxelBody(const Voxel& v, program& p, const btTransform& worldTransform, bool dynamic) {
        auto m = v.makeMesh();
        buf.bind(GL_ARRAY_BUFFER).data(GL_ARRAY_BUFFER, m->vertices, GL_STATIC_DRAW);
        vao.bind();
        VoxelVertex::describe().use();
        elm.bind(GL_ELEMENT_ARRAY_BUFFER).data(GL_ELEMENT_ARRAY_BUFFER, m->elements, GL_STATIC_DRAW);
        count = (GLsizei) m->elements.size();
        
        btCompoundShape* colShape = new btCompoundShape(true); // leak!
		// collisionShapes.push_back(colShape);
        btTransform childTransform;
		childTransform.setIdentity();
        vector<btScalar> masses;
        btScalar totalMass = 0;
        for (int i = 0; i != v.size()[0]; ++i)
            for (int j = 0; j != v.size()[1]; ++j)
                for (int k = 0; k != v.size()[2]; ++k)
                    if (v(i,j,k))
                    {
                        childTransform.setOrigin(btVector3(i + 0.5, j + 0.5, k + 0.5));
                        colShape->addChildShape(childTransform, new btBoxShape(btVector3(0.5,0.5,0.5))); // leak!
                        masses.push_back(1);
                        totalMass += 1;
                    }
        btTransform principal;
        btVector3 localInertia;
        colShape->calculatePrincipalAxisTransform(masses.data(), principal, localInertia);
        for (int i = 0; i != colShape->getNumChildShapes(); ++i) {
            btTransform c = colShape->getChildTransform(i);
            colShape->updateChildTransform(i, principal.inverseTimes(c));
        }
        if (!dynamic) {
            totalMass = 0;
            localInertia = btVector3(0,0,0);
        }
        btDefaultMotionState* myMotionState = new btDefaultMotionState(worldTransform * principal);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(totalMass,myMotionState,colShape,localInertia);
        body.reset(new btRigidBody(rbInfo));
        principal.inverse().getOpenGLMatrix(model.data());

    }
    
    void draw(program& p) {
        p.use();
        vao.bind();
        elm.bind(GL_ELEMENT_ARRAY_BUFFER);
     
        btTransform trans;
        body->getMotionState()->getWorldTransform(trans);
        
        mat<float, 4, 4> a;
        trans.getOpenGLMatrix(a.data());
        a = a * model;
        p["modelMatrix"] = a;
        p["inverseTransposeModelMatrix"] = inverse(transpose(a));
        
        
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
        
        
    }

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
    
    unique_ptr<VoxelBody> vb;
    unique_ptr<VoxelBody> ground;
    
    unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
	unique_ptr<btCollisionDispatcher> dispatcher;
	unique_ptr<btBroadphaseInterface> overlappingPairCache;
	unique_ptr<btSequentialImpulseConstraintSolver> solver;
	unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
    btAlignedObjectArray<btCollisionShape*> collisionShapes;

    
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
    
    
    
    
    
    
    
    glEnable(GL_CULL_FACE);
    opengl_health();

    p->use();
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

    
    /*
    Voxel v{vec<size_t,3>{3,2,1}};
    v(0,0,0) = v(0, 1, 0) = v(1, 1, 0) = v(2, 1, 0) = 1;
     */
    
    Voxel v{vec<size_t,3>{4,4,1}};
    randomize(v);
    
    
    btTransform trans;
    trans.setIdentity();
    vb.reset(new VoxelBody(v, *p, trans, true));
    dynamicsWorld->addRigidBody(vb->body.get());
    
    trans.setOrigin(btVector3(0,-5,0));
    ground.reset(new VoxelBody(v, *p, trans, false));
    dynamicsWorld->addRigidBody(ground->body.get());
    
    
    
    
    
    
    
    p->validate();
    p->use();
    
    
    // Bullet init end
    
    
   
    
    
    
    
    
}

my_application::~my_application() {
    
    //remove the rigidbodies from the dynamics world and delete them
	for (int i=dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject( obj );
		delete obj;
	}
    
	//delete collision shapes
	for (int j=0;j<collisionShapes.size();j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}
    
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
    
    /*
    //print positions of all objects
    for (int j=dynamicsWorld->getNumCollisionObjects()-1; j>=0 ;j--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            btTransform trans;
            body->getMotionState()->getWorldTransform(trans);
            
            mat<float, 4, 4> model;
            trans.getOpenGLMatrix(model.data());
            model = model * translate(vec<float, 3>(-0.5f, -0.5f, -0.5f));
            (*p)["modelMatrix"] = model;
            (*p)["inverseTransposeModelMatrix"] = inverse(transpose(model));
            
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
            
            
            
            
            
            
        }
    }
     */
    
    vb->draw(*p);
    ground->draw(*p);
    

    opengl_health();
    
    
}