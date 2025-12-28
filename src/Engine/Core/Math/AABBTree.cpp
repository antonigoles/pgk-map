#include "Engine/Core/Math/MinMaxQueue.hpp"
#include <Engine/Core/Math/AABBTree.hpp>
#include <vector>
#include <iostream>

#define MAX_AABB_TREE_NODE_SIZE 64
#define AABB_JOIN_THRESHOLD 16
#define AABB_MARGIN 0.1f

namespace Engine {
	bool AABB_vec_contains(
		const glm::vec3& minP,
		const glm::vec3& maxP,
		const glm::vec3& vec
	) {
		return (vec.x >= minP.x && vec.y >= minP.y && vec.z >= minP.y)
			&& (vec.x <= maxP.x && vec.y <= maxP.y && vec.z <= maxP.y);
	}

	bool AABB_trig_contains(
		const glm::vec3& minP,
		const glm::vec3& maxP,
		const glm::vec3 vec[3]
	) {
		return AABB_vec_contains(minP, maxP, vec[0]) &&
				AABB_vec_contains(minP, maxP, vec[1]) &&
				AABB_vec_contains(minP, maxP, vec[2]);
	};

	AABBTreeElement::AABBTreeElement(glm::vec3 vert[3]) {
		this->vert[0] = vert[0];
		this->vert[1] = vert[1];
		this->vert[2] = vert[2];
	}

	AABBTreeNode::AABBTreeNode(int depth) {
		this->depth = depth;
		childA=nullptr;
		childB=nullptr;
		parent=nullptr;
		xCoords = MinMaxQueue<float>();
		yCoords = MinMaxQueue<float>();
		zCoords = MinMaxQueue<float>();
		std::cout << "Creating node with depth " << depth << "\n";
	}

	void push_to_queues(
		MinMaxQueue<float>& xCoords, 
		MinMaxQueue<float>& yCoords, 
		MinMaxQueue<float>& zCoords,
		const glm::vec3& vert
	) {
		xCoords.insert(vert.x);
		yCoords.insert(vert.y);
		zCoords.insert(vert.z);
	}

	void remove_from_queues(
		MinMaxQueue<float>& xCoords, 
		MinMaxQueue<float>& yCoords, 
		MinMaxQueue<float>& zCoords,
		const glm::vec3& vert
	) {
		xCoords.remove(vert.x);
		yCoords.remove(vert.y);
		zCoords.remove(vert.z);
	}

	glm::vec3 AABBTreeNode::getMaxP() {
		if (this->childA!=nullptr) {
			auto aMinP = this->childA->getMaxP();
			auto bMinP = this->childB->getMaxP();
			return glm::vec3{
				std::max(aMinP.x, bMinP.x) + AABB_MARGIN,
				std::max(aMinP.y, bMinP.y) + AABB_MARGIN,
				std::max(aMinP.z, bMinP.z) + AABB_MARGIN
			};
		}
		if (elements.size() == 0) return {0,0,0};
		return glm::vec3{
			this->xCoords.getMax() + AABB_MARGIN,
			this->yCoords.getMax() + AABB_MARGIN,
			this->zCoords.getMax() + AABB_MARGIN
		};
	};
	
	glm::vec3 AABBTreeNode::getMinP() {
		if (this->childA!=nullptr) {
			auto aMinP = this->childA->getMinP();
			auto bMinP = this->childB->getMinP();
			return glm::vec3{
				std::min(aMinP.x, bMinP.x) - AABB_MARGIN,
				std::min(aMinP.y, bMinP.y) - AABB_MARGIN,
				std::min(aMinP.z, bMinP.z) - AABB_MARGIN
			};
		}
		if (elements.size() == 0) return {0,0,0};
		return glm::vec3{
			this->xCoords.getMin() - AABB_MARGIN,
			this->yCoords.getMin() - AABB_MARGIN,
			this->zCoords.getMin() - AABB_MARGIN
		};
	};

	void AABBTreeNode::lazyInsert(AABBTreeElement* element) {
		this->elements.insert(element);
		element->node = this;
		push_to_queues(xCoords, yCoords, zCoords, element->vert[0]);
		push_to_queues(xCoords, yCoords, zCoords, element->vert[1]);
		push_to_queues(xCoords, yCoords, zCoords, element->vert[2]);
		// extendAABB_trig(this->minP, this->maxP, element->vert);
	};

	void AABBTreeNode::remove(AABBTreeElement* element) {
		this->elements.erase(element);
		remove_from_queues(xCoords, yCoords, zCoords, element->vert[0]);
		remove_from_queues(xCoords, yCoords, zCoords, element->vert[1]);
		remove_from_queues(xCoords, yCoords, zCoords, element->vert[2]);
		if (this->elements.size() < AABB_JOIN_THRESHOLD && this->childA != nullptr) {
			if (this->parent != nullptr) this->parent->join();
		}
	};


	void AABBTreeNode::insert(AABBTreeElement* element) {
		if (this->childA != nullptr) {
			// I'm not a leaf anymore, need to pass this down
			bool aContain = AABB_trig_contains(this->childA->getMinP(), 
				this->childA->getMaxP(), element->vert);

			bool bContain = AABB_trig_contains(this->childB->getMinP(), 
				this->childB->getMaxP(), element->vert);

			// if either both contain this triangle or neither of them
			if (aContain && bContain || (!aContain && !bContain)) {
				if (this->childA->elements.size() > this->childB->elements.size()) {
					this->childB->insert(element);
				} else {
					this->childA->insert(element);
				}
			}
			
			if (aContain && !bContain) {
				this->childA->insert(element);
			}

			if (bContain && !aContain) {
				this->childB->insert(element);
			}
		} else if (this->elements.size() > MAX_AABB_TREE_NODE_SIZE) {
			this->split();
			this->insert(element);
			return;
		} else {
			this->elements.insert(element);
			element->node = this;
			push_to_queues(xCoords, yCoords, zCoords, element->vert[0]);
			push_to_queues(xCoords, yCoords, zCoords, element->vert[1]);
			push_to_queues(xCoords, yCoords, zCoords, element->vert[2]);
		}
	};

	void AABBTreeNode::join() {
		std::cout << "Joining children\n";
		for (auto element : this->childA->elements) {
			this->lazyInsert(element);
		}

		for (auto element : this->childB->elements) {
			this->lazyInsert(element);
		}

		delete this->childA;
		this->childA = nullptr;
		delete this->childB;
		this->childB = nullptr;
	};

	void AABBTreeNode::split() {
		// hmm how to implement this quickly?
		// Idea #1: Random sampling - todo: benchmark : / not a fan
		// Idea #2: Just split by 3 middle axis and check

		if (this->splitStrategyFailure) return;
		if (this->elements.size() <= MAX_AABB_TREE_NODE_SIZE) return;

		glm::vec3 mid = glm::vec3{
			this->xCoords.getMid(), 
			this->yCoords.getMid(), 
			this->zCoords.getMid()
		};

		std::vector<int> countBy = {0,0,0};

		for (auto e : this->elements) {
			if (e->vert[0].x < mid.x && e->vert[1].x < mid.x && e->vert[2].x < mid.x) countBy[0]++;
			if (e->vert[0].y < mid.y && e->vert[1].y < mid.y && e->vert[2].y < mid.y) countBy[1]++;
			if (e->vert[0].z < mid.z && e->vert[1].z < mid.z && e->vert[2].z < mid.z) countBy[2]++;
		}

		countBy[0]=abs((int)(countBy[0]-this->elements.size()/2));
		countBy[1]=abs((int)(countBy[1]-this->elements.size()/2));
		countBy[2]=abs((int)(countBy[2]-this->elements.size()/2));


		int minValue = std::min(countBy[0], std::min(countBy[1], countBy[2]));
		float splitFactor = 1.0f - ((float) minValue / (float)(this->elements.size()/2.0f));

		if (splitFactor <= 0.3f) {
			this->splitStrategyFailure = true;
			return;
		}

		this->childA = new AABBTreeNode(this->depth+1);
		this->childB = new AABBTreeNode(this->depth+1);
		this->childA->parent = this;
		this->childB->parent = this;

		if (countBy[2] >= countBy[0] && countBy[1] >= countBy[0]) {
			// Use X
			for (auto e : this->elements) {
				if (e->vert[0].x < mid.x && e->vert[1].x < mid.x && e->vert[2].x < mid.x) {
					this->childA->lazyInsert(e);
				} else {
					this->childB->lazyInsert(e);
				}
			}
		} else if (countBy[0] >= countBy[1] && countBy[2] >= countBy[1]) {
			// Use Y
			for (auto e : this->elements) {
				if (e->vert[0].y < mid.y && e->vert[1].y < mid.y && e->vert[2].y < mid.y) {
					this->childA->lazyInsert(e);
				} else {
					this->childB->lazyInsert(e);
				}
			}
		} else {
			// Use Z
			for (auto e : this->elements) {
				if (e->vert[0].z < mid.z && e->vert[1].z < mid.z && e->vert[2].z < mid.z) {
					this->childA->lazyInsert(e);
				} else {
					this->childB->lazyInsert(e);
				}
			}
		}

		this->elements.clear();
		this->xCoords.clear();
		this->yCoords.clear();
		this->zCoords.clear();

		this->childA->split();
		this->childB->split();
	};


	AABBTree::AABBTree() {
		root = nullptr;
	}

	std::unordered_set<AABBTreeNode*> AABBTree::getAllLeafNodes() {
		std::unordered_set<AABBTreeNode*> nodes;
		for (auto p : this->objectToElementVec) {
			for (auto e : p.second) {
				nodes.insert(e->node);
			}
		}
		return nodes;
	};

	std::unordered_set<AABBTreeNode*> AABBTree::getNodesById(int id) {
		std::unordered_set<AABBTreeNode*> nodes;
		for (auto e : this->objectToElementVec[id]) {
			nodes.insert(e->node);
		}
		return nodes;
	};

	const std::vector<AABBTreeElement*>& AABBTree::getElementsById(int id) {
		return this->objectToElementVec[id];
	};

	int AABBTree::addObject(Mesh* mesh, glm::mat4 transform) {
		int id = nextID++;
		this->oldTransforms[id] = transform;

		float* data = mesh->get_data();
		for (int i = 0; i<mesh->size(); i+=24) {
			glm::vec3 trig[3] = {
				glm::vec3(transform * glm::vec4{data[i], data[i+1], data[i+2], 1.0f}),
				glm::vec3(transform * glm::vec4{data[i+8], data[i+9], data[i+10], 1.0f}),
				glm::vec3(transform * glm::vec4{data[i+16], data[i+17], data[i+18], 1.0f})
			};
			AABBTreeElement* el = new AABBTreeElement(trig);
			this->objectToElementVec[id].push_back(el);
		}

		return id;
	}

	void AABBTree::updateObject(int objectId, glm::mat4 transform) {
		// lazy

		std::set<AABBTreeNode*> nodesToBallance;
		glm::mat4 fullTrans = transform * glm::inverse(this->oldTransforms[objectId]);
		for (auto& e : this->objectToElementVec[objectId]) {
			e->node->remove(e);
			e->vert[0] = glm::vec3(fullTrans * glm::vec4(e->vert[0], 1.0f));
			e->vert[1] = glm::vec3(fullTrans * glm::vec4(e->vert[1], 1.0f));
			e->vert[2] = glm::vec3(fullTrans * glm::vec4(e->vert[2], 1.0f));
			this->root->insert(e);
		}
		this->oldTransforms[objectId] = transform;
	}

	void AABBTree::buildTree() {
		root = new AABBTreeNode(0);
		// push everything first and withhold splitting before that

		for (auto vec : this->objectToElementVec) {
			for (auto el : vec.second) {
				root->lazyInsert(el);
			}
		}
		std::cout << "Building AABBTree of size " << root->elements.size() << "\n";
		root->split();
	}
}