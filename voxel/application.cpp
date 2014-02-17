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
#include "framebuffer.h"

using namespace std;

application::~application() {
}






class buffer : public Named {
public:
    buffer() { glGenBuffers(1, &name_); }
    ~buffer() { glDeleteBuffers(1, &name_); }
    buffer& bind(GLenum target) { glBindBuffer(target, name_); return *this;}
    template<typename T> buffer& data(GLenum target, vector<T> data, GLenum usage) {
        glBufferData(target, sizeof(T) * data.size(), data.data(), usage);
        return *this;
    }
};


class vertex_array : public Named {
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
    
    VoxelBody(const Voxel& v, program& p, const btTransform& worldTransform, bool dynamic, btDynamicsWorld& dynamicsWorld) {
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
                        // should check if this voxel is surrounded and therefore not necessary ///////////////////////////////////
                        childTransform.setOrigin(btVector3(i + 0.5, j + 0.5, k + 0.5));
                        colShape->addChildShape(childTransform, new btBoxShape(btVector3(0.5,0.5,0.5))); // leak! /////////////////
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
        dynamicsWorld.addRigidBody(body.get());
        principal.inverse().getOpenGLMatrix(model.data());

    }
    
    ~VoxelBody() {
        // undo add to dynamics world ///////////////////////////////////////////////////////////
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

class Projection {
public:
    mat<float, 4, 4> proj;
    mat<float, 4, 4> view;
    void drawAsCamera(program& p) {
        p["cameraProjectionMatrix"] = proj;
        p["cameraViewMatrix"] = view;
    }
    void drawAsSpotlight(program& p) {
        p["spotlightProjectionMatrix"] = proj;
        p["spotlightViewMatrix"] = view;
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
    unique_ptr<ShadowBuffer> shadowBuffer;
    
    Projection camera;
    Projection light;
    
    vector<unique_ptr<VoxelBody>> bodies;
    
    
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
    glActiveTexture(GL_TEXTURE2);
    shadowBuffer.reset(new ShadowBuffer(4096, 4096));

    
    
    opengl_health();
    
    
    
    
    
    
    
    glEnable(GL_CULL_FACE);
    opengl_health();

    p->use();
    (*p)["textureMatrix"] = identity<float, 4>() * (1.0f/16.f);
    (*p)["samplerColor"] = 0;
    (*p)["samplerNormal"] = 1;
    (*p)["samplerShadow"] = 2;
    
    
    
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
    
    {
        btCollisionShape* groundShape = new btBoxShape(btVector3(50,50,50));
        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0,-50,0));
        btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(0,myMotionState,groundShape,btVector3(0,0,0));
        btRigidBody* body = new btRigidBody(rbInfo);
        
        //add the body to the dynamics world
        dynamicsWorld->addRigidBody(body);

    }
    
    
    Voxel v{vec<size_t,3>{16,16,16}};
    randomize(v);
    
    VoxelPainting blobs = v.paint();
    for (short a : blobs.unique) {
        Voxel u(v.size());
        // selectively copy from v into u
        for (size_t k = 0; k != v.size()[2]; ++k)
            for (size_t j = 0; j != v.size()[1]; ++j)
                for (size_t i = 0; i != v.size()[0]; ++i)
                    if (blobs.mapping[blobs.p(i,j,k)] == a)
                        u(i, j, k) = v(i, j, k);
        // make a new object
        btTransform trans;
        trans.setIdentity();
        bodies.emplace_back(new VoxelBody(u, *p, trans, true, *dynamicsWorld));
    }
    
    
    
    //randomize(v);
    
    //trans.setOrigin(btVector3(0,-5,0));
    //ground.reset(new VoxelBody(v, *p, trans, false, *dynamicsWorld));
    
    
    
    
    
    
    
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
    
    static int countdown = 60;
    
    if (--countdown < 0)
        dynamicsWorld->stepSimulation(1.f/60.f,10);
    
    camera.proj = perspective((float) M_PI_4*0.7f, width / (float) height, 1.f, 1000.f);
    camera.view = lookat(vec<float, 3>(32.0f, 48.0f, 64.0f),
                         vec<float, 3>(8.0f, 8.0f, 8.0f),
                         vec<float, 3>(0.0f, 1.0f, 0.0f));
    light.proj = perspective((float) M_PI_4*0.7f, width / (float) height, 1.f, 1000.f);
    light.view = lookat(vec<float, 3>(48.0f, 64.0f, 32.0f),
                        vec<float, 3>(8.0f, 8.0f, 8.0f),
                        vec<float, 3>(0.0f, 1.0f, 0.0f));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *textureColor);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, *textureNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    shadowBuffer->bind();
    glViewport(0, 0, shadowBuffer->width, shadowBuffer->height);
    glClear(GL_DEPTH_BUFFER_BIT);
    light.drawAsCamera(*p);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2, 1);
    for (auto& q : bodies)
        q->draw(*p);
    glDisable(GL_POLYGON_OFFSET_FILL);

    glBindTexture(GL_TEXTURE_2D, shadowBuffer->depth);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, (int) width, (int) height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.drawAsCamera(*p);
    light.drawAsSpotlight(*p);
    for (auto& q : bodies)
        q->draw(*p);
    
    opengl_health();
    
    
}