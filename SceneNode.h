#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT
#include <xnamath.h>
#include <dinput.h>
#include <vector>
#include "Model.h"

class SceneNode
{
private:
	Model* m_p_model;
	vector<SceneNode*> m_children;
	float m_x, m_y, m_z;
	float m_dx, m_dz, m_rotation;
	float m_xangle, m_zangle, m_yangle;
	float m_scale;
	float m_world_centre_x, m_world_centre_y, m_world_centre_z, m_world_scale;

public:
	SceneNode();
	void setPosition(float x, float y, float z);
	void setRotation(float x, float y, float z);
	void setScale(float value);
	void setModel(Model* aModel);
	float getPositionX();
	float getPositionY();
	float getPositionZ();
	float getRotation();
	float getScale();
	void MoveForward(float distance);
	bool MoveForward(float distance, SceneNode* root_node);
	void MoveRight(float distance);
	bool MoveRight(float distance, SceneNode* root_node);
	void addChildNode(SceneNode *n);
	bool detatchNode(SceneNode *n);
	void execute(XMMATRIX *world, XMMATRIX* view, XMMATRIX* projection);
	XMVECTOR  get_world_centre_position();
	void update_collision_tree(XMMATRIX* world, float scale);
	bool check_collision(SceneNode* compare_tree);
	bool check_collision(SceneNode* compare_tree, SceneNode* object_tree_root);
	ObjFileModel* getObject();
	XMMATRIX m_local_world_matrix;

	void check_collision_ray(ObjFileModel::xyz* rayPosition, ObjFileModel::xyz* directionRay);

};