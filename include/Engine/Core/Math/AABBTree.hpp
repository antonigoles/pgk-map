#pragma once
#include <Engine/Core/OpenGL.hpp>
#include <Engine/Core/Mesh.hpp>
#include <unordered_set>
#include <Engine/Core/Math/MinMaxQueue.hpp>

namespace Engine {
	class AABBTreeNode;
	class AABBTreeElement;
	class AABBTree;

	/*
	The idea:
	- For each node we store 2 children
	- At one node there can be at most U amount of nodes
	- Each element stored is a 2D triangle
	- Element will be inserted at the lowest child which bounds can conatin the entire triangle
	- "Split" will be decided searching through triangles in a node and deciding 
	- "Semi-Offline by design" -> most element will not update during runtime
	*/

	/* Internal usage only */
	class AABBTreeElement {
	public:
		AABBTreeElement(glm::vec3 vert[3]);
		glm::vec3 vert[3];

		AABBTreeNode* node;
	};

	class AABBTreeNode {
	private:
		int depth;
		AABBTreeNode *parent;
		AABBTreeNode *childA;
		AABBTreeNode *childB;

		bool splitStrategyFailure=false;
	public:
		glm::vec3 getMaxP();
		glm::vec3 getMinP();

		MinMaxQueue<float> xCoords;
		MinMaxQueue<float> yCoords;
		MinMaxQueue<float> zCoords;

		std::unordered_set<AABBTreeElement*> elements;
		AABBTreeNode(int depth);
		void insert(AABBTreeElement* element);
		void remove(AABBTreeElement* element);

		void lazyInsert(AABBTreeElement* element);

		void split();
		void join();
	};


	class AABBTree {
	private:
		// we want to be able to quickly fetch Node by triangle, and element by Mesh
		std::unordered_map<int, std::vector<AABBTreeElement*>> objectToElementVec; // id -> element vector
		std::unordered_map<int, glm::mat4> oldTransforms; // id -> element vector
		AABBTreeNode* root;

		int nextID=0;
	public:
		AABBTree();
		
		// returns object id
		int addObject(Mesh* mesh, glm::mat4 transform);
		void updateObject(int objectId, glm::mat4 transform);

		std::unordered_set<AABBTreeNode*> getNodesById(int id);
		std::unordered_set<AABBTreeNode*> getAllLeafNodes();

		const std::vector<AABBTreeElement*>& getElementsById(int id);

		void buildTree();
	}; 
}