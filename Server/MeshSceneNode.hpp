#pragma once

class MeshSceneNodeNotify;

class MeshSceneNode : public SceneNode, public ndBodyDynamic
{

	std::string Model;
public:
	MeshSceneNode(std::string Model, ndVector Position);
	~MeshSceneNode();

    void Tick();
};

class MeshSceneNodeNotify : public ndBodyNotify
{
    MeshSceneNode* _Node;

public:
    MeshSceneNodeNotify(MeshSceneNode* Node)
        : _Node(Node), ndBodyNotify(ndVector(0.0f, -10.0f, 0.0f, 0.0f))
    {}

    void* GetUserData() const
    {
        return (void*)_Node;
    }

    void OnApplyExternalForce(ndInt32, ndFloat32)
    {
        ndBodyDynamic* const dynamicBody = GetBody()->GetAsBodyDynamic();
        if (dynamicBody)
        {
            ndVector massMatrix(dynamicBody->GetMassMatrix());
            //ndVector force(ndVector(0.0f, -10.0f, 0.0f, 0.0f).Scale(massMatrix.m_w));
            ndVector force(dynamicBody->GetNotifyCallback()->GetGravity().Scale(massMatrix.m_w));
            dynamicBody->SetForce(force);
            dynamicBody->SetTorque(ndVector::m_zero);
        }
    }

    void OnTransform(ndInt32 threadIndex, const ndMatrix& matrix)
    {
        const ndVector Pos = matrix.m_posit;
        //_Node->_Camera->SetPosition(glm::vec3(Pos.m_x, Pos.m_y, Pos.m_z) + _Node->_Camera->getOffset());
        wxLogMessage("[MeshSceneNode] OnTransform");
    }
};

MeshSceneNode::MeshSceneNode(std::string Model, ndVector Position) :
    SceneNode(Position), ndBodyDynamic(), Model(Model)
{

    GLTFInfo* Infos = WorldEngine::SceneGraph::LoadModel(Model.c_str());
    std::vector<ndVector> Vertices;
    Vertices.reserve(Infos->Indices.size());
    for (unsigned int i = 0; i < Infos->Indices.size(); i++)
    {
        auto& V = Infos->Vertices[Infos->Indices[i]].pos;
        Vertices.push_back(ndVector(V.x, V.y, V.z, 1.0f));
    }

    ndShapeInstance Shape(new ndShapeConvexHull((ndInt32)Vertices.size(), sizeof(ndVector), 0.0f, &Vertices[0].m_x));

    ndMatrix Matrix(dGetIdentityMatrix());
    Matrix.m_posit = Position;
    Matrix.m_posit.m_w = 1.0f;  //  Ensure W == 1 because Newton wont work otherwise.

    SetNotifyCallback(new MeshSceneNodeNotify(this));
    SetMatrix(Matrix);
    SetCollisionShape(Shape);
    mass = 10.0f;
    WorldEngine::_ndWorld->Sync();
    WorldEngine::_ndWorld->AddBody(this);

    //
    //	Send initial connection packet
    //KNet::NetPacket_Send* Pkt1 = _Client->GetFreePacket<KNet::ChannelID::Reliable_Ordered>();
    //if (Pkt1) {
    //    Pkt1->write<unsigned int>(0);									//	0 == Initial Connection Info
    //    Pkt1->write<bool>(true);										//	true = Identify as 'server'
    //    Pkt1->write<const char*>(WorldEngine::CurrentMap.c_str());		//	Current Map File
    //    Pkt1->write<const char*>(Model.c_str());					//	Player Model File
    //    Pkt1->write<float>(Position.GetX());							//	Player Position - X
    //    Pkt1->write<float>(Position.GetY());							//	Player Position - Y
    //    Pkt1->write<float>(Position.GetZ());							//	Player Position - Z
    //    _Point->SendPacket(Pkt1);
    //    wxLogMessage("[Player] Send Initial Packet");
    //}
    //else { printf("PKT1 UNAVAILABLE!\n"); }
}

MeshSceneNode::~MeshSceneNode()
{
    WorldEngine::GetPhysicsWorld()->RemoveBody(this);
    wxLogMessage("[MeshSceneNode] Deleted!");
}

void MeshSceneNode::Tick()
{
}