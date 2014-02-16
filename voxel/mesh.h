//
//  mesh.h
//  voxel
//
//  Created by Antony Searle on 2/11/14.
//  Copyright (c) 2014 Antony Searle. All rights reserved.
//

#ifndef voxel_mesh_h
#define voxel_mesh_h

using std::vector;
using std::unique_ptr;

template<typename V, typename E> class mesh {
public:
    std::vector<V> vertices;
    std::vector<E> elements;
    /*
    typedef typename V::position_type position_type;
    
    void add_position(position_type a) {
        V b;
        b.position = a;
        vertices.push_back(b);
    }
    
    void add_triangle(position_type a, position_type b, position_type c) {
        size_t i = vertices.size();
        elements.push_back(i);
        elements.push_back(i+1);
        elements.push_back(i+2);
        add_position(a);
        add_position(b);
        add_position(c);
    }

    void add_quad(position_type a, position_type b, position_type c, position_type d) {
        size_t i = vertices.size();
        elements.push_back((E) i);
        elements.push_back((E) i+1);
        elements.push_back((E) i+2);
        elements.push_back((E) i);
        elements.push_back((E) i+2);
        elements.push_back((E) i+3);
        add_position(a);
        add_position(b);
        add_position(c);
        add_position(d);
    }
        
    static std::unique_ptr<mesh> make_cube() {
        std::unique_ptr<mesh> m{new mesh{}};
        typedef position_type v;
        
        m->add_quad(v(0,0,0), v(0,0,1), v(0,1,1), v(0,1,0)); // -X
        m->add_quad(v(0,0,0), v(1,0,0), v(1,0,1), v(0,0,1)); // -Y
        m->add_quad(v(0,0,0), v(0,1,0), v(1,1,0), v(1,0,0)); // -Z
        
        m->add_quad(v(0,0,1), v(1,0,1), v(1,1,1), v(0,1,1)); // +X
        m->add_quad(v(0,1,0), v(0,1,1), v(1,1,1), v(1,1,0)); // +Y
        m->add_quad(v(1,0,0), v(1,1,0), v(1,1,1), v(1,0,1)); // +Z
        
        return m;
    }
     */
};


#endif
