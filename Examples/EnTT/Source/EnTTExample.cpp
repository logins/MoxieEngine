/*
 EnTTExample.cpp

 Moxie Engine - https://github.com/logins/MoxieEngine

 MIT License - Copyright (c) 2021 Riccardo Loggini
*/
#include <entt/entity/registry.hpp>

#include <entt/entt.hpp>

#include <iostream>
/*
void MyFreeFunction() { }

class MyMonitoringClass { public: void Member(){} };

class RenderableComponent { public: void Init(){} };


using offset_t = uint16_t;

#define PTR_OFFSET_SZ sizeof(offset_t)

#ifndef align_up
#define align_up(InSize, InAlignment) \
    ((InSize + InAlignment - 1) & ~(InAlignment - 1))
#endif

	void* aligned_malloc(size_t InSize, size_t InAlignment)
	{
		void* alignedPtr = nullptr;

		if (InSize && InAlignment)
		{
			if ((InAlignment & (InAlignment - 1)) != 0) // Checking if alignment is power of 2
				return nullptr;

			size_t hdrSize = (InAlignment - 1) + PTR_OFFSET_SZ; // Worst case for header size

			void* p = malloc(hdrSize + InSize);

			std::cout << "Malloc pointer " << p << std::endl;

			if (p)
			{
				alignedPtr = (void*)align_up(((uintptr_t)p + PTR_OFFSET_SZ), (InAlignment));

				std::cout << "Aligned pointer " << alignedPtr << std::endl;

	*((offset_t*)alignedPtr - 1) = (offset_t)((uintptr_t)alignedPtr - (uintptr_t)p);
			}
		}

		return alignedPtr;
	}

	void aligned_free(void* InPtr)
	{
		// Assuming we are passing a pointer given by aligned_malloc
		offset_t offset = *((offset_t*)InPtr - 1);

		free((uint8_t*)InPtr - offset);
	}

	struct MyStruct
	{
		int a; char b; float c;
	};
*/
	/*
	std::cout << "Align of MyStruct: " << alignof(MyStruct) << " Bytes" << std::endl;


	void* myAlignedMalloc = aligned_malloc(sizeof(MyStruct), 32768);

	*((MyStruct*)myAlignedMalloc) = { 1, '2', 3.f };

	aligned_free(myAlignedMalloc);
	*/

void main()
{
	// A registry creates entities, views and groups
	entt::registry registry;

	// ----- CREATE ENTITIES

	// constructs a naked entity with no components and returns its identifier
	auto entity = registry.create();


	// ----- CREATE AND ASSIGN COMPONENTS

	struct Position { int32_t x; int32_t y; };

	registry.emplace<Position>(entity, 0, 0);
	// Note: A component can also be a class! 

	struct Velocity { float dx; float dy; };

	auto& vel = registry.emplace<Velocity>(entity);
	vel.dx = 0.;
	vel.dy = 0.;

	auto entity2 = registry.create();

	// default initialized type assigned by copy to all entities
	//registry.insert<Position>(entity, entity2);

	// user-defined instance assigned by copy to all entities
	//registry.insert(entity, entity2, Position{ 0, 0 });

	// first and last specify the range of entities, instances points to the first element of the range of components
	//registry.insert<Position>(entity, entity2, componentsRange);

	// replaces the component in-place
	registry.patch<Position>(entity, [](auto& pos) { pos.x = pos.y = 0.; });

	// constructs a new instance from a list of arguments and replaces the component
	registry.replace<Position>(entity, 0, 0);

	// when we don't know if the component was created or not
	registry.emplace_or_replace<Position>(entity, 0, 0);

	// remove a component IF exists
	registry.remove<Position>(entity);

	// clear all the component instances
	registry.clear<Position>();
/*
	// ----- QUERY COMPONENTS

	// true if entity has all the given components
	bool all = registry.all_of<Position, Velocity>(entity);

	// true if entity has at least one of the given components
	bool any = registry.any_of<Position, Velocity>(entity);

	const auto& cregistry = registry;
	// const and non-const references
	const auto [cpos, cvel] = cregistry.get<Position, Velocity>(entity);
	auto [pos, vel2] = registry.get<Position, Velocity>(entity);

	// ----- DESTROY ENTITIES

	// destroys an entity and all its components
	registry.destroy(entity);

	// destroys all the entities, with Position and Velocity as components, in a specified range
	auto view = registry.view<Position, Velocity>();
	registry.destroy(view.begin(), view.end());

	// destroys all the entities at once
	registry.clear();

	// ----- QUERY ENTITIES

	// returns true if the entity is still valid, false otherwise
	bool b = registry.valid(entity);

	// gets the version contained in the entity identifier
	auto version = registry.version(entity);

	// gets the actual version for the given entity
	auto curr = registry.current(entity);

	// const and non-const reference
	const auto& crenderable = cregistry.get<Position>(entity);
	auto& renderableComponent = registry.get<Position>(entity);

	// ----- OBSERVE CHANGES

	// connects a free function
	registry.on_construct<Position>().connect<&MyFreeFunction>();

	MyMonitoringClass instance;

	// connects a member function
	registry.on_construct<Position>().connect<&MyMonitoringClass::Member>(instance);

	// disconnects a free function
	registry.on_construct<Position>().disconnect<&MyFreeFunction>();

	// disconnects a member function
	registry.on_construct<Position>().disconnect<&MyMonitoringClass::Member>(instance);

	// ----- OBSERVER OBJECTS

	// This observer will reference all the entities inside registry that changed their Position component
	entt::observer observer{ registry, entt::collector.update<Position>() };
	// Note: Updates from the Collector are based on calls to the on_update method, which gets called when we call patch on the entities components!
	
	// We can also use the where clause to specify more conditions
	struct Sprite { int32_t x; int32_t y; };
	entt::collector.update<Sprite>().where<Position>(entt::exclude<Velocity>);

	// Then we can fetch the referenced entities with
	for (const auto entity : observer) {
		// ...
	}
	observer.clear();
	// Or we can use the each function, that will ALSO clear the observer automatically (if it is not a const observer)
	observer.each([](const auto entity) {
		// ...
		});

	// Invoke a component member function as a callback
	//registry.on_construct<RenderableComponent>().connect<entt::invoke<&RenderableComponent::Init>>();

	// ----- VIEWS
	
	// Views are used to iterate over components
	// single component view
	auto single = registry.view<Position>();

	// multi component view
	auto multi = registry.view<Position, Velocity>();

	auto view = registry.view<Position, Velocity>(entt::exclude<RenderableComponent>);

	auto view = registry.view<Position, Velocity, RenderableComponent>();

	for (auto entity : view) {
		// a component at a time ...
		auto& position = view.get<Position>(entity);
		auto& velocity = view.get<Velocity>(entity);

		// ... multiple components ...
		auto [pos3, vel3] = view.get<Position, Velocity>(entity);

		// ... all components at once
		//auto [pos4, vel4, rend4] = view.get(entity);

		// through a callback
		registry.view<position, velocity>().each([](auto entity, auto& pos, auto& vel) {
			// ...
			});

		// using an input iterator
		for (auto&& [entity, pos, vel] : registry.view<position, velocity>().each()) {
			// ...
		}

		auto view = registry.view<const RenderableComponent>();

		for (auto entity : view) {
			auto [RenderableComponent] = view.get(entity);
			// ...
		}
	}
	*/

system("pause");

}