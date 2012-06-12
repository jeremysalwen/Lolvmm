/*
  Copyright 2007-2011 David Robillard <http://drobilla.net>
            2012      Jeremy Salwen <jeremysalwen@gmail.com>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef LOLV_LOLVMM_HPP
#define LOLV_LOLVMM_HPP

#include "lilv/lilv.h"
#include "memory"

namespace Lolv {

inline const char* uri_to_path(const char* uri) { return lilv_uri_to_path(uri); }

//naming scheme is
//LOLV_[CONST]_WRAP_X_Y_Z_...
//
//Where X is the return type, and Y, Z, etc are the argument types.
//the CONST is optionally in there, if the (this) parameter is const.
//
//The valid types are V for Void (only for a return type)
//                    O for lilv Object.
//                    CO for a Const lilv Object.  
//                    P for Primitive type (int, etc, where no wrapping or unwrapping has to be done).
//
//So for example 
// LOLV_WRAP_CO_P_O
//inside Lolv::World will wrap a function which looks like 
// const LilvNode* lilv_world_thingy(LilvWorld* w, int i, LilvNode* n)
//into
// const Node* Lolv::World::thingy(int i, Node* n)

#define LOLV_WRAP_V(prefix, name) \
	inline void name() { lilv_ ## prefix ## _ ## name(Unwrap(this)); }

#define LOLV_WRAP_V_O(prefix, name, T1, a1) \
	inline void name(T1* a1) { lilv_ ## prefix ## _ ## name(Unwrap(this),T1::Unwrap(a1)); }

#define LOLV_WRAP_V_P(prefix, name, T1, a1) \
	inline void name(T1 a1) { lilv_ ## prefix ## _ ## name(Unwrap(this),a1); }

#define LOLV_WRAP_V_P_CO(prefix, name, T1, a1, T2, a2) \
	inline void name(T1 a1, const T2* a2) { lilv_ ## prefix ## _ ## name(Unwrap(this),a1,T2::ConstUnwrap(a2)); }

#define LOLV_WRAP_V_P_P(prefix, name, T1, a1, T2, a2) \
	inline void name(T1 a1, T2 a2) { lilv_ ## prefix ## _ ## name(Unwrap(this),a1,a2); }

#define LOLV_CONST_WRAP_V_P_P_P(prefix, name, T1, a1, T2, a2, T3, a3) \
	inline void name(T1 a1, T2 a2, T3 a3) const { lilv_ ## prefix ## _ ## name(ConstUnwrap(this),a1,a2,a3); }


#define LOLV_WRAP_O(RT, RTprefix, prefix, name) \
	inline std::unique_ptr<RT, std::function<void(RT*)>> name() { \
		auto deleter=[&](RT* n) { \
				lilv_ ## RTprefix ## _free(RT::Unwrap(n));\
			}; \
		return std::unique_ptr<RT,decltype(deleter)>( \
			RT::Wrap(lilv_ ## prefix ## _ ## name (Unwrap(this))), \
			deleter); \
	}

#define LOLV_WRAP_O_CO_CO_CO(RT, RTprefix, prefix, name, T1, a1, T2, a2, T3, a3) \
	inline std::unique_ptr<RT, std::function<void(RT*)>> name (const T1* a1, const T2* a2, const T3* a3) { \
		auto deleter=[&](RT* n) { \
				lilv_ ## RTprefix ## _free(RT::Unwrap(n));\
			}; \
		return std::unique_ptr<RT, std::function<void(RT*)>>( \
			RT::Wrap(lilv_ ## prefix ## _ ## name (Unwrap(this),T1::ConstUnwrap(a1),T2::ConstUnwrap(a2),T3::ConstUnwrap(a3))), \
			deleter); \
	}

#define LOLV_WRAP_O_P(RT, RTprefix, prefix, name, T1, a1) \
	inline std::unique_ptr<RT, std::function<void(RT*)>> name (T1 a1) { \
		auto deleter=[&](RT* n) { \
				lilv_ ## RTprefix ## _free(RT::Unwrap(n));\
			}; \
		return std::unique_ptr<RT,decltype(deleter)>( \
			RT::Wrap(lilv_ ## prefix ## _ ## name (Unwrap(this),a1)), \
			deleter); \
	}

#define LOLV_CONST_WRAP_O(RT, RTprefix, prefix, name) \
	inline std::unique_ptr<RT, std::function<void(RT*)>> name() const { \
		auto deleter=[&](RT* n) { \
				lilv_ ## RTprefix ## _free(RT::Unwrap(n));\
			}; \
		return std::unique_ptr<RT,decltype(deleter)>( \
			RT::Wrap(lilv_ ## prefix ## _ ## name (ConstUnwrap(this))), \
			deleter); \
	}

#define LOLV_CONST_WRAP_O_CO(RT, RTprefix, prefix, name, T1, a1) \
	inline std::unique_ptr<RT, std::function<void(RT*)>> name (const T1* a1) { \
		auto deleter=[&](RT* n) { \
				lilv_ ## RTprefix ## _free(RT::Unwrap(n));\
			}; \
		return std::unique_ptr<RT,decltype(deleter)>( \
			RT::Wrap(lilv_ ## prefix ## _ ## name (ConstUnwrap(this),T1::ConstUnwrap(a1))), \
			deleter); \
	}


#define LOLV_CONST_WRAP_O_P_P(RT, RTprefix, prefix, name, T1, a1, T2, a2) \
	inline std::unique_ptr<RT, std::function<void(RT*)>> name (T1 a1, T2 a2) const { \
		auto deleter=[&](RT* n) { \
				lilv_ ## RTprefix ## _free(reinterpret_cast<Lilv ## RT *>(n));\
			}; \
		return std::unique_ptr<RT,decltype(deleter)>( \
			reinterpret_cast<RT*>(lilv_ ## prefix ## _ ## name (ConstUnwrap(this),a1,a2)), \
			deleter); \
	}


#define LOLV_WRAP_CO(RT, prefix, name) \
	inline const RT* name() { return RT::ConstWrap(lilv_ ## prefix ## _ ## name (Unwrap(this))); }

#define LOLV_CONST_WRAP_CO(RT, prefix, name) \
	inline const RT* name() const { return RT::ConstWrap(lilv_ ## prefix ## _ ## name (ConstUnwrap(this))); }

#define LOLV_CONST_WRAP_CO_CO(RT, prefix, name, T1, a1) \
	inline const RT* name(const T1* a1) const { return reinterpret_cast<const RT*>(lilv_ ## prefix ## _ ## name (ConstUnwrap(this),T1::ConstUnwrap(a1))); }

#define LOLV_CONST_WRAP_CO_P(RT, prefix, name, T1, a1) \
	inline const RT* name(T1 a1) const { return reinterpret_cast<const RT*>(lilv_ ## prefix ## _ ## name (ConstUnwrap(this),a1)); }


#define LOLV_WRAP_P(RT, prefix, name) \
	inline RT name() { return lilv_ ## prefix ## _ ## name (Unwrap(this)); }

#define LOLV_CONST_WRAP_P(RT, prefix, name) \
	inline RT name() const { return lilv_ ## prefix ## _ ## name (ConstUnwrap(this)); }

#define LOLV_CONST_WRAP_P_CO(RT, prefix, name, T1, a1) \
	inline RT name(const T1* a1) const { return lilv_ ## prefix ## _ ## name (ConstUnwrap(this),T1::ConstUnwrap(a1)); }

#define LOLV_CONST_WRAP_P_P(RT, prefix, name, T1, a1) \
	inline RT name(T1 a1) const { return lilv_ ## prefix ## _ ## name (ConstUnwrap(this),a1); }



#define LOLV_DEFINE_CONST_UNWRAP(T) \
	static inline const Lilv ## T * ConstUnwrap(const T* node) {return reinterpret_cast<const Lilv ## T *>(node); } \

#define LOLV_DEFINE_CONST_WRAP(T) \
	static inline const T* ConstWrap(const Lilv ## T * node) {return reinterpret_cast<const T*>(node); } \
	LOLV_DEFINE_CONST_UNWRAP(T)

#define LOLV_DEFINE_UNWRAP(T) \
	static inline Lilv ## T * Unwrap(T* node) {return reinterpret_cast<Lilv ## T *>(node); } \

#define LOLV_DEFINE_WRAP(T) \
	static inline T* Wrap(Lilv ## T * node) {return reinterpret_cast<T*>(node); } \
	LOLV_DEFINE_CONST_WRAP(T)\
	LOLV_DEFINE_UNWRAP(T)
	
// Cannot be copied, assigned, or constructed
#define LOLV_HIDE_CREATION(T) \
	T(); \
	T(T const&); \
	T& operator=(T const&);

struct Node {
	LOLV_DEFINE_WRAP(Node)

	LOLV_CONST_WRAP_O(Node, node,  node, duplicate);
	LOLV_CONST_WRAP_P(char*,       node, get_turtle_token);
	LOLV_CONST_WRAP_P(bool,        node, is_uri);
	LOLV_CONST_WRAP_P(const char*, node, as_uri);
	LOLV_CONST_WRAP_P(bool,        node, is_blank);
	LOLV_CONST_WRAP_P(const char*, node, as_blank);
	LOLV_CONST_WRAP_P(bool,        node, is_literal);
	LOLV_CONST_WRAP_P(bool,        node, is_string);
	LOLV_CONST_WRAP_P(const char*, node, as_string);
	LOLV_CONST_WRAP_P(bool,        node, is_float);
	LOLV_CONST_WRAP_P(float,       node, as_float);
	LOLV_CONST_WRAP_P(bool,        node, is_int);
	LOLV_CONST_WRAP_P(int,         node, as_int);
	LOLV_CONST_WRAP_P(bool,        node, is_bool);
	LOLV_CONST_WRAP_P(bool,        node, as_bool);

	inline bool equals(const Node& other) const { return lilv_node_equals(ConstUnwrap(this),ConstUnwrap(&other)); }
	inline bool operator==(const Node& other) const { return equals(other); }

 private:
	LOLV_HIDE_CREATION(Node)
};

struct ScalePoint {
   	LOLV_DEFINE_CONST_WRAP(ScalePoint);
	
	LOLV_CONST_WRAP_CO(Node, scale_point, get_label);
	LOLV_CONST_WRAP_CO(Node, scale_point, get_value);
 private:
	LOLV_HIDE_CREATION(ScalePoint)
};

typedef LilvIter Iter;

struct PluginClass;

#define LOLV_WRAP_COLL(CT, ET, prefix) \
    	LOLV_DEFINE_WRAP(CT); \
	LOLV_CONST_WRAP_P(unsigned, prefix, size); \
	LOLV_CONST_WRAP_CO_P(ET, prefix, get, Iter*, i); \
	LOLV_CONST_WRAP_P(Iter*, prefix, begin); \
	LOLV_CONST_WRAP_P_P(Iter*, prefix, next, Iter*, i); \
	LOLV_CONST_WRAP_P_P(bool, prefix, is_end, Iter*, i); \
  private: \
	LOLV_HIDE_CREATION(CT)

struct PluginClasses {
	LOLV_CONST_WRAP_CO_CO(PluginClass, plugin_classes, get_by_uri, Node, uri);
	LOLV_WRAP_COLL(PluginClasses, PluginClass, plugin_classes);
};


struct PluginClass{
	LOLV_DEFINE_CONST_WRAP(PluginClass);
	
	LOLV_CONST_WRAP_CO(Node, plugin_class, get_parent_uri);
	LOLV_CONST_WRAP_CO(Node, plugin_class, get_uri);
	LOLV_CONST_WRAP_CO(Node, plugin_class, get_label);
	LOLV_CONST_WRAP_O(PluginClasses, plugin_classes, plugin_class, get_children);
private:
	LOLV_HIDE_CREATION(PluginClass)
};


struct ScalePoints {
	LOLV_WRAP_COLL(ScalePoints, ScalePoint, scale_points);
};

struct Nodes {
	LOLV_CONST_WRAP_P_CO(bool, nodes, contains, Node, node);
	LOLV_WRAP_COLL(Nodes, Node, nodes);
};
		
struct Port {
	LOLV_DEFINE_CONST_WRAP(Port);
 private:
	LOLV_HIDE_CREATION(Port)
};
struct Plugin;

struct PortRef {
#define LOLV_PORT_WRAP_O(RT, RTprefix, name) \
	inline std::unique_ptr<RT,std::function<void(RT*)>> name () const { \
		auto deleter=[&](RT* n) { lilv_ ## RTprefix ## _free(RT::Unwrap(n)); }; \
		return std::unique_ptr<RT, std::function<void(RT*)>>( \
			RT::Wrap(lilv_port_ ## name ( \
				reinterpret_cast<const LilvPlugin*>(parent), \
				Port::ConstUnwrap(port))), \
			deleter); \
	}

#define LOLV_PORT_WRAP_O_CO(RT, RTprefix, name, T1, a1) \
	inline std::unique_ptr<RT,std::function<void(RT*)>> name (const T1* a1) const { \
		auto deleter=[&](RT* n) { lilv_ ## RTprefix ## _free(RT::Unwrap(n)); }; \
		return std::unique_ptr<RT, std::function<void(RT*)>>( \
			RT::Wrap(lilv_port_ ## name ( \
				reinterpret_cast<const LilvPlugin*>(parent), \
				Port::ConstUnwrap(port), \
				T1::ConstUnwrap(a1))), \
			deleter); \
	}

#define LOLV_PORT_WRAP_CO(RT, name) \
	inline const RT* name () const { return RT::ConstWrap(lilv_port_ ## name (reinterpret_cast<const LilvPlugin*>(parent),  Port::ConstUnwrap(port))); }

#define LOLV_PORT_WRAP_P_CO(RT, name, T1, a1) \
	inline RT name (const T1* a1) const { return lilv_port_ ## name (reinterpret_cast<const LilvPlugin*>(parent),  Port::ConstUnwrap(port), T1::ConstUnwrap(a1)); }

	LOLV_PORT_WRAP_O_CO(Nodes, nodes, get_value, Node, predicate);
	LOLV_PORT_WRAP_O(Nodes, nodes, get_properties)
	LOLV_PORT_WRAP_P_CO(bool, has_property, Node, property_uri);
	LOLV_PORT_WRAP_P_CO(bool, supports_event, Node, event_uri);
	LOLV_PORT_WRAP_CO(Node,  get_symbol);
	LOLV_PORT_WRAP_O(Node,  node, get_name);
	LOLV_PORT_WRAP_CO(Nodes, get_classes);
	LOLV_PORT_WRAP_P_CO(bool, is_a, Node, port_class);
	LOLV_PORT_WRAP_O(ScalePoints, scale_points, get_scale_points);
	
	inline void get_range(Node ** deflt, Node ** min, Node ** max) {
		lilv_port_get_range(reinterpret_cast<const LilvPlugin*>(parent),Port::ConstUnwrap(port),reinterpret_cast<LilvNode**>(deflt),reinterpret_cast<LilvNode**>(min),reinterpret_cast<LilvNode**>(max));
	}
	const Plugin* parent;
	const Port* port;
};

struct Instance;

struct Plugin {
	LOLV_DEFINE_CONST_WRAP(Plugin)
	LOLV_CONST_WRAP_P(bool,      		plugin, verify);
	LOLV_CONST_WRAP_CO(Node,     		plugin, get_uri);
	LOLV_CONST_WRAP_CO(Node,   		plugin, get_bundle_uri);
	LOLV_CONST_WRAP_CO(Nodes,		plugin, get_data_uris);
	LOLV_CONST_WRAP_CO(Node,  		plugin, get_library_uri);
	LOLV_CONST_WRAP_O(Node,		node, 	plugin, get_name);
	LOLV_CONST_WRAP_CO(PluginClass, 	plugin, get_class);
	LOLV_CONST_WRAP_O_CO(Nodes, 	nodes, 	plugin, get_value, 		Node, pred);
	LOLV_CONST_WRAP_P_CO(bool,		plugin, has_feature, 		Node, feature_uri);
	LOLV_CONST_WRAP_O(Nodes, 	nodes,	plugin, get_supported_features);
	LOLV_CONST_WRAP_O(Nodes, 	nodes,	plugin, get_required_features);
	LOLV_CONST_WRAP_O(Nodes, 	nodes,	plugin, get_optional_features);
	LOLV_CONST_WRAP_P(uint32_t,   		plugin, get_num_ports);
	LOLV_CONST_WRAP_P(bool,        		plugin, has_latency);
	LOLV_CONST_WRAP_P(uint32_t,    		plugin, get_latency_port_index);
	LOLV_CONST_WRAP_O(Node,		node, 	plugin, get_author_name);
	LOLV_CONST_WRAP_O(Node,		node,	plugin, get_author_email);
	LOLV_CONST_WRAP_O(Node,		node,	plugin, get_author_homepage);
	LOLV_CONST_WRAP_P(bool,			plugin, is_replaced);
   	LOLV_CONST_WRAP_O_P_P(Instance,	instance,plugin,instantiate,		double, sample_rate, 	const LV2_Feature *const *, features);
	
	inline PortRef get_port_by_index(unsigned index) {
		PortRef result={this, Port::ConstWrap(lilv_plugin_get_port_by_index(ConstUnwrap(this), index))};
		return result;
	}

	inline PortRef get_port_by_symbol(const Node* symbol) {
		PortRef result={this, Port::ConstWrap(lilv_plugin_get_port_by_symbol(ConstUnwrap(this), Node::ConstUnwrap(symbol)))};
		return result;
	}

	LOLV_CONST_WRAP_V_P_P_P(plugin, get_port_ranges_float, float*, min_values, float*, max_values, float*, def_values);


	inline unsigned get_num_ports_of_class(LilvNode* class_1,
	                                       LilvNode* class_2) {
		// TODO: varargs
		return lilv_plugin_get_num_ports_of_class(ConstUnwrap(this), class_1, class_2, NULL);
	}
 private:
	LOLV_HIDE_CREATION(Plugin)
};

struct Plugins {
	LOLV_CONST_WRAP_CO_CO(Plugin, plugins, get_by_uri, Node, uri);
	LOLV_WRAP_COLL(Plugins, Plugin, plugins);
};

struct Instance {
	LOLV_DEFINE_WRAP(Instance);

	LOLV_WRAP_V_P_P(instance, connect_port,
	                unsigned, port_index,
	                void*,    data_location);

	LOLV_WRAP_V(instance, activate);
	LOLV_WRAP_V_P(instance, run, unsigned, sample_count);
	LOLV_WRAP_V(instance, deactivate);
	LOLV_CONST_WRAP_P_P(const void*, instance, get_extension_data, const char*, uri);
	LOLV_CONST_WRAP_P(const LV2_Descriptor*, instance, get_descriptor);
 private:
	LOLV_HIDE_CREATION(Instance)
};

struct World {
	static inline std::unique_ptr<World, std::function<void(World*)>> NewWorld() {
		auto deleter=[&](World* n) { lilv_world_free(World::Unwrap(n));};
		return std::unique_ptr<World, std::function<void(World*)>>(World::Wrap(lilv_world_new()),deleter);
	}
	
	LOLV_DEFINE_WRAP(World);
	
#define LOLV_WRAP_WORLD_NEW(T,suffix) \
	inline std::unique_ptr<Node> new_ ## suffix(T val) { \
		auto deleter=[&](Node* n) { lilv_node_free(Node::Unwrap(n));}; \
		return std::unique_ptr<Node>(Node::Wrap(lilv_new_ ## suffix(Unwrap(this), val))); \
	}
	
	LOLV_WRAP_WORLD_NEW(const char*,uri);
	LOLV_WRAP_WORLD_NEW(const char*,string);
	LOLV_WRAP_WORLD_NEW(int,int);
	LOLV_WRAP_WORLD_NEW(float,float);
	LOLV_WRAP_WORLD_NEW(bool,bool);
	
	LOLV_WRAP_O_CO_CO_CO(Nodes, nodes, world, find_nodes, Node, subject, Node, predicate, Node, object);

	LOLV_WRAP_V_P_CO(world, set_option, const char*, uri, Node, value);
	LOLV_WRAP_V(world, load_all);
	LOLV_WRAP_V_O(world, load_bundle, Node, bundle_uri);
	LOLV_CONST_WRAP_CO(PluginClass, world, get_plugin_class);
	LOLV_CONST_WRAP_CO(PluginClasses, world, get_plugin_classes);
	LOLV_CONST_WRAP_CO(Plugins, world, get_all_plugins);
 private:
	LOLV_HIDE_CREATION(World)
};

} /* namespace Lilv */


#endif /* LOLV_LOLVMM_HPP */
