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
#include <map>

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
#include "Buffer.h"

using namespace std;

application::~application() {
}






class VoxelBody {
public:
    mat<float, 4, 4> ATTRIBUTE_ALIGNED128(model);
    ArrayBuffer buf;
    ElementArrayBuffer elm;
    VertexArrayObject vao;
    GLsizei count;
    unique_ptr<btRigidBody> body;
    unique_ptr<btCompoundShape> colShape;
    btDynamicsWorld* m_dynamicsWorld;
    shared_ptr<const Voxel> m_voxel;
    
    static btBoxShape* unitCube() {
        static unique_ptr<btBoxShape> p{new btBoxShape{btVector3{0.5,0.5,0.5}}};
        return p.get();
    }
    
    VoxelBody(shared_ptr<const Voxel> v, program& p, const btTransform& worldTransform, bool dynamic, btDynamicsWorld& dynamicsWorld) {
        
        // Set up drawing code
        
        m_voxel = v;
        auto m = v->makeMesh();
        buf.data(m->vertices);
        vao.bind();
        VoxelVertex::describe().use();
        elm.data(m->elements);
        count = (GLsizei) m->elements.size();
        
        // Populate compound shape with occupied voxels
        
        colShape.reset(new btCompoundShape(true));
		btTransform childTransform;
		childTransform.setIdentity();
        vector<btScalar> masses;
        btScalar totalMass = 0;
        for (int i = 0; i != v->size()[0]; ++i)
            for (int j = 0; j != v->size()[1]; ++j)
                for (int k = 0; k != v->size()[2]; ++k)
                    if ((*v)(i,j,k))
                    {
                        // should check if this voxel is surrounded and therefore does not participate in collision detection
                        // though it must be there to get mass and inertia tensor right
                        
                        // should merge adjacent voxels -- run length encoding -- to produce vastly fewer objects
                        // combine anything with the same density, not same identity
                        
                        childTransform.setOrigin(btVector3(i + 0.5, j + 0.5, k + 0.5));
                        colShape->addChildShape(childTransform, unitCube());
                        masses.push_back(1);
                        totalMass += 1;
                    }
        
        // Determine the principal axes and transform to them
        
        btTransform principal;
        btVector3 localInertia;
        colShape->calculatePrincipalAxisTransform(masses.data(), principal, localInertia);
        for (int i = 0; i != colShape->getNumChildShapes(); ++i) {
            btTransform c = colShape->getChildTransform(i);
            colShape->updateChildTransform(i, principal.inverseTimes(c));
        }
        principal.inverse().getOpenGLMatrix(model.data());
        
        // Create rigid body and place it in the world
        
        if (!dynamic) {
            totalMass = 0;
            localInertia = btVector3(0,0,0);
        }
        btDefaultMotionState* myMotionState = new btDefaultMotionState(worldTransform * principal);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(totalMass,myMotionState,colShape.get(),localInertia);
        body.reset(new btRigidBody(rbInfo));
        dynamicsWorld.addRigidBody(body.get());
        m_dynamicsWorld = &dynamicsWorld;

    }
    
    ~VoxelBody() {
        // undo add to dynamics world ///////////////////////////////////////////////////////////
        m_dynamicsWorld->removeRigidBody(body.get());
    }
    
    void draw(program& p) {
        p.use();
        vao.bind();
        elm.bind();
     
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


int64_t morton(int64_t x, int64_t y, int64_t z) {
    int64_t r = 0;
    for (int i = 0; i != 21; ++i) {
        r |= (x & (1 << i)) << (i * 3 + 0);
        r |= (y & (1 << i)) << (i * 3 + 1);
        r |= (z & (1 << i)) << (i * 3 + 2);
    }
    return r;
}

template<typename T, size_t chunkSize> class ChunkedVoxel {
    
    struct Chunk {
        vector<T> data_;
        //Chunk() : data_(chunkSize * chunkSize * chunkSize) {}
        Chunk(int64_t x, int64_t y, int64_t z) : data_(chunkSize * chunkSize * chunkSize) {
            x *= chunkSize;
            y *= chunkSize;
            z *= chunkSize;
        }
        T getVoxel(int64_t x, int64_t y, int64_t z) {
            return data_[x + chunkSize * (y + chunkSize * z)];
        }
        void setVoxel(int64_t x, int64_t y, int64_t z, T t) {
            data_[x + chunkSize * (y + chunkSize * z)] = t;
        }
    };
    
    map<int64_t, Chunk> map_;
    
    
    T getVoxel(int64_t x, int64_t y, int64_t z) {
        int64_t m = morton(x/chunkSize, y/chunkSize, z/chunkSize);
        auto i = map_.find(m);
        return i != map_.end() ? map_->get(x%chunkSize,y%chunkSize,z%chunkSize) : 0;
    }
    
    void setVoxel(int64_t x, int64_t y, int64_t z, T t) {
        int64_t m = morton(x/chunkSize, y/chunkSize, z/chunkSize);
        auto i = map_.find(m);
        if (i == map_.end) {
            i = map_.emplace(x/chunkSize,y/chunkSize,z/chunkSize).first;
        }
        return i->set(x % chunkSize,
                      y % chunkSize,
                      z % chunkSize,
                      t);
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

    virtual void mouseDragged(float deltaX, float deltaY);
    virtual void mouseLocation(float mouseX, float mouseY);

private:
    
    float phi, theta;
    float mouseX, mouseY;
    
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
    
    phi = -0.5;
    theta = 0.5;
    
    p = make_program("basic");
    
    opengl_health();
    
    glActiveTexture(GL_TEXTURE0);
    textureColor = makeTexture(*makeImageTextures());
    glActiveTexture(GL_TEXTURE1);
    textureNormal = makeTexture(*makeImageNormals());
    glActiveTexture(GL_TEXTURE2);
    shadowBuffer.reset(new ShadowBuffer(2048, 2048));

    
    
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
        btCollisionShape* groundShape = new btBoxShape(btVector3(32,32,32));
        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0,-32,0));
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
        auto u = make_shared<Voxel>(v.size());
        // selectively copy from v into u
        for (size_t k = 0; k != v.size()[2]; ++k)
            for (size_t j = 0; j != v.size()[1]; ++j)
                for (size_t i = 0; i != v.size()[0]; ++i)
                    if (blobs.mapping[blobs.p(i,j,k)] == a)
                        (*u)(i, j, k) = v(i, j, k);
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
    
}

void my_application::mouseLocation(float mouseX, float mouseY) {
    this->mouseX = mouseX;
    this->mouseY = mouseY;
}

void my_application::mouseDragged(float deltaX, float deltaY) {
    // make thread safe
    phi += deltaX * 0.01f;
    theta += deltaY * 0.01f;
}


void my_application::render(size_t width, size_t height, double time) {
    
    
    
    // Retina means the mouse coordinates in points don't match the pixels
    float u = mouseX / width * 2 - 1;
    float v = mouseY / height * 2 - 1;
    vec<float, 4> start = inverse(camera.proj * camera.view) * vec<float, 4>(u,v,-1,1);
    start /= start.w;
    vec<float, 4> end = inverse(camera.proj * camera.view) * vec<float, 4>(u,v,+1,1);
    end /= end.w;

    btVector3 rayFromWorld(start.x, start.y, start.z);
    btVector3 rayToWorld(end.x, end.y, end.z);
    btCollisionWorld::ClosestRayResultCallback RayCallback(rayFromWorld, rayToWorld);
    dynamicsWorld->getCollisionWorld()->rayTest(rayFromWorld, rayToWorld, RayCallback);
    if (RayCallback.hasHit()) {
        btRigidBody* body = const_cast<btRigidBody*>(btRigidBody::upcast(RayCallback.m_collisionObject));
        if (body) {
            //body->applyCentralForce(btVector3(0,20,0));
            //body->applyCentralForce(RayCallback.m_hitNormalWorld * -100);
            //body->applyForce(RayCallback.m_hitNormalWorld / body->getInvMass() * 10,
            //                 body->getWorldTransform().inverse() * RayCallback.m_hitPointWorld);
            //body->applyForce((rayFromWorld - rayToWorld).normalize() / body->getInvMass() * -10,
            //                 body->getWorldTransform().inverse() * RayCallback.m_hitPointWorld);
            body->applyForce((rayFromWorld - rayToWorld).normalize() * -1000,
                             body->getWorldTransform().inverse() * RayCallback.m_hitPointWorld);
            body->setActivationState(DISABLE_DEACTIVATION);
        }
    }
    
    
    dynamicsWorld->stepSimulation(1.f/60.f,10);
    
    camera.proj = perspective((float) M_PI_4*0.7f, width / (float) height, 1.f, 1000.f);
    /*
    camera.view = lookat(vec<float, 3>(32.0f, 48.0f, 64.0f),
                         vec<float, 3>(8.0f, 8.0f, 8.0f),
                         vec<float, 3>(0.0f, 1.0f, 0.0f));
     */
    camera.view = translate(vec<float, 3>(0,0,-70)) * rotateX(theta) * rotateY(phi) * translate(vec<float,3>(-8.f,0.f,-8.f));
    
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