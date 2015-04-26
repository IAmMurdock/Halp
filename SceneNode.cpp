#include "SceneNode.h"

SceneNode::SceneNode()
{
	m_p_model = NULL;

	m_x, m_y, m_z = 0.0f;
	m_dx, m_dz, m_rotation = 0.0f;
	m_xangle, m_zangle, m_yangle = 0.0f;
	m_scale = 1.0f;


}

void SceneNode::setModel(Model* aModel)
{
	m_p_model = aModel;
}

void SceneNode::setPosition(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}
void SceneNode::setRotation(float x, float y, float z)
{
	m_xangle = x;
	m_zangle = y;
	m_yangle = z;
}
void SceneNode::setScale(float value)
{
	m_scale = value;
}
float SceneNode::getPositionX()
{
	return m_x;
}
float SceneNode::getPositionY()
{
	return m_y;
}
float SceneNode::getPositionZ()
{
	return m_z;
}
float SceneNode::getScale()
{
	return m_scale;
}

void SceneNode::addChildNode(SceneNode *n)
{
	m_children.push_back(n);
}

void SceneNode::MoveForward(float distance)
{
	m_x += sin(m_yangle * (XM_PI / 180.0f))*distance;
	m_z += cos(m_yangle * (XM_PI / 180.0f))*distance;
}

bool SceneNode::MoveForward(float distance, SceneNode* root_node )
{
	float old_x = m_x;
	float old_z = m_z;

	m_x += sin(m_yangle * (XM_PI / 180.0f))*distance;
	m_z += cos(m_yangle * (XM_PI / 180.0f))*distance;

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);

	// check for collision of this node (and children) against all other nodes
	if (check_collision(root_node) == true)
	{
		// if collision restore state
		m_x = old_x;

		return true;
	}

	return false;

}

void SceneNode::MoveRight(float distance)
{
	m_z += sin(m_yangle * (XM_PI / 180.0f))*distance;
	m_x += cos(m_yangle * (XM_PI / 180.0f))*distance;
}

bool SceneNode::MoveRight(float distance, SceneNode* root_node)
{
	float old_x = m_x;
	float old_z = m_z;

	m_z += sin(m_yangle * (XM_PI / 180.0f))*distance;
	m_x += cos(m_yangle * (XM_PI / 180.0f))*distance;

	XMMATRIX identity = XMMatrixIdentity();

	// since state has changed, need to update collision tree
	// this basic system requires entire hirearchy to be updated
	// so start at root node passing in identity matrix
	root_node->update_collision_tree(&identity, 1.0);

	// check for collision of this node (and children) against all other nodes
	if (check_collision(root_node) == true)
	{
		// if collision restore state
		m_z = old_z;

		return true;
	}

	return false;

}

bool SceneNode::detatchNode(SceneNode*n)
{
	// traverse tree to find node to detatch
	for (int i = 0; i < m_children.size(); i++)
	{
		if (n == m_children[i])
		{
			m_children.erase(m_children.begin() + i);
			return true;
		}
		if (m_children[i]->detatchNode(n) == true) return true;
	}
	return false; // node not in this tree
}

void SceneNode::execute(XMMATRIX *world, XMMATRIX* view, XMMATRIX* projection)
{
	// the local_world matrix will be used to calc the local transformations for this node
	XMMATRIX local_world = XMMatrixIdentity();

	local_world = XMMatrixRotationX(XMConvertToRadians(m_xangle));
	local_world *= XMMatrixRotationY(XMConvertToRadians(m_yangle));
	local_world *= XMMatrixRotationZ(XMConvertToRadians(m_zangle));

	local_world *= XMMatrixScaling(m_scale, m_scale, m_scale);

	local_world *= XMMatrixTranslation(m_x, m_y, m_z);

	// the local matrix is multiplied by the passed in world matrix that contains the concatenated
	// transformations of all parent nodes so that this nodes transformations are relative to those
	local_world *= *world;

	// only draw if there is a model attached
	if (m_p_model) m_p_model->draw(world, view, projection);

	// traverse all child nodes, passing in the concatenated world matrix
	for (int i = 0; i< m_children.size(); i++)
	{
		m_children[i]->execute(&local_world, view, projection);
	}
}

XMVECTOR  SceneNode::get_world_centre_position()
{
	return XMVectorSet(m_world_centre_x,
		m_world_centre_y,
		m_world_centre_z, 0.0);
}

void  SceneNode::update_collision_tree(XMMATRIX* world, float scale)
{
	// the m_local_world_matrix matrix will be used to calculate the local transformations for this node
	XMMATRIX m_local_world_matrix = XMMatrixIdentity();

	m_local_world_matrix = XMMatrixRotationX(XMConvertToRadians(m_xangle));
	m_local_world_matrix *= XMMatrixRotationY(XMConvertToRadians(m_yangle));
	m_local_world_matrix *= XMMatrixRotationZ(XMConvertToRadians(m_zangle));

	m_local_world_matrix *= XMMatrixScaling(m_scale, m_scale, m_scale);

	m_local_world_matrix *= XMMatrixTranslation(m_x, m_y, m_z);

	// the local matrix is multiplied by the passed in world matrix that contains the concatenated
	// transformations of all parent nodes so that this nodes transformations are relative to those
	m_local_world_matrix *= *world;

	// calc the world space scale of this object, is needed to calculate the  
	// correct bounding sphere radius of an object in a scaled hierarchy
	m_world_scale = scale * m_scale;

	XMVECTOR v;
	if (m_p_model)
	{
		v = XMVectorSet(m_p_model->GetBoundingSphere_x(),
			m_p_model->GetBoundingSphere_y(),
			m_p_model->GetBoundingSphere_z(), 0.0);
	}
	else v = XMVectorSet(0, 0, 0, 0); // no model, default to 0

	// find and store world space bounding sphere centre
	v = XMVector3Transform(v, m_local_world_matrix);
	m_world_centre_x = XMVectorGetX(v);
	m_world_centre_y = XMVectorGetY(v);
	m_world_centre_z = XMVectorGetZ(v);

	// traverse all child nodes, passing in the concatenated world matrix and scale
	for (int i = 0; i< m_children.size(); i++)
	{
		m_children[i]->update_collision_tree(&m_local_world_matrix, m_world_scale);
	}
}

bool SceneNode::check_collision(SceneNode* compare_tree)
{
	return check_collision(compare_tree, this);
}

bool SceneNode::check_collision(SceneNode* compare_tree, SceneNode* object_tree_root)
{
	// check to see if root of tree being compared is same as root node of object tree being checked
	// i.e. stop object node and children being checked against each other
	if (object_tree_root == compare_tree) return false;

	// only check for collisions if both nodes contain a model
	if (m_p_model && compare_tree->m_p_model)
	{
		XMVECTOR v1 = get_world_centre_position();
		XMVECTOR v2 = compare_tree->get_world_centre_position();
		XMVECTOR vdiff = v1 - v2;

		//XMVECTOR a = XMVector3Length(vdiff);
		float x1 = XMVectorGetX(v1);
		float x2 = XMVectorGetX(v2);
		float y1 = XMVectorGetY(v1);
		float y2 = XMVectorGetY(v2);
		float z1 = XMVectorGetZ(v1);
		float z2 = XMVectorGetZ(v2);

		float dx = x1 - x2;
		float dy = y1 - y2;
		float dz = z1 - z2;

		// check bounding sphere collision
		if (sqrt(dx*dx + dy*dy + dz*dz) <
			(compare_tree->m_p_model->GetBoundingSphereRadius() * compare_tree->m_world_scale) +
			(this->m_p_model->GetBoundingSphereRadius() * m_world_scale))
		{
			return true;
		}
	}

	// iterate through compared tree child nodes
	for (int i = 0; i< compare_tree->m_children.size(); i++)
	{
		// check for collsion against all compared tree child nodes 
		if (check_collision(compare_tree->m_children[i], object_tree_root) == true) return true;
	}

	// iterate through composite object child nodes
	for (int i = 0; i< m_children.size(); i++)
	{
		// check all the child nodes of the composite object against compared tree
		if (m_children[i]->check_collision(compare_tree, object_tree_root) == true) return true;
	}

	return false;

}

ObjFileModel* SceneNode::getObject()
{

	return m_p_model->m_pObject;

}

void SceneNode::check_collision_ray(ObjFileModel::xyz* rayPosition, ObjFileModel::xyz* directionRay)
{
	XMVECTOR rayPos, worldCentre, distance;
	rayPos.x = rayPosition->x;
	rayPos.y = rayPosition->y;
	rayPos.z = rayPosition->z;

	worldCentre = get_world_centre_position();

	distance = rayPos - worldCentre;



		
}