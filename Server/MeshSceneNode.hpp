#pragma once

class MeshSceneNodeNotify;

class MeshSceneNode : public SceneNode, public ndBodyDynamic
{
	std::string Model;
public:
    ndMatrix Matrix;
	MeshSceneNode(std::string Model, ndVector Position);
	~MeshSceneNode();

    void Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime);
    std::chrono::time_point<std::chrono::steady_clock> LastUpdate = std::chrono::high_resolution_clock::now();
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
    }
};

MeshSceneNode::MeshSceneNode(std::string Model, ndVector Position) :
    SceneNode(Position), ndBodyDynamic(), Model(Model), Matrix(ndGetIdentityMatrix())
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

    Matrix.m_posit = Position;
    Matrix.m_posit.m_w = 1.0f;  //  Ensure W == 1 because Newton wont work otherwise.

    SetNotifyCallback(new MeshSceneNodeNotify(this));
    SetMatrix(Matrix);
    SetCollisionShape(Shape);
    mass = 10.0f;
    SetMassMatrix(mass, Shape);
    SetAngularDamping(ndVector(1.f, 1.f, 1.f, 1.f));
    SetLinearDamping(0.1f);
    WorldEngine::_ndWorld->Sync();
    WorldEngine::_ndWorld->AddBody(this);
}

MeshSceneNode::~MeshSceneNode()
{
    WorldEngine::GetPhysicsWorld()->RemoveBody(this);
    wxLogMessage("[MeshSceneNode] Deleted!");
}

void MeshSceneNode::Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime)
{
    if (LastUpdate + std::chrono::milliseconds(250) < CurTime)
    {
        ndVector Velocity = GetVelocity();
        Matrix = GetMatrix();
        
        for (auto& Client : WorldEngine::NetCode::ConnectedClients)
        {
            KNet::NetPacket_Send* Pkt = Client.first->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Update_SceneNode);
            if (Pkt)
            {
                Pkt->write<uintmax_t>(GetNodeID());                                                         //  SceneNode ID
                Pkt->write<float>(Matrix.m_posit.m_x);                                                      //  Position - X
                Pkt->write<float>(Matrix.m_posit.m_y);	                                                      //    Position - Y
                Pkt->write<float>(Matrix.m_posit.m_z);	                                                      //	Position - Z
                Pkt->write<float>(Matrix.m_posit.m_w);	                                                      //	Position - W
                Pkt->write<float>(Matrix.m_front.m_x);	                                                      //	Front - X
                Pkt->write<float>(Matrix.m_front.m_y);	                                                      //	Front - Y
                Pkt->write<float>(Matrix.m_front.m_z);	                                                      //	Front - Z
                Pkt->write<float>(Matrix.m_front.m_w);	                                                      //	Front - W
                Pkt->write<float>(Matrix.m_right.m_x);	                                                      //	Right - X
                Pkt->write<float>(Matrix.m_right.m_y);	                                                      //	Right - Y
                Pkt->write<float>(Matrix.m_right.m_z);	                                                      //	Right - Z
                Pkt->write<float>(Matrix.m_right.m_w);	                                                      //	Right - W
                Pkt->write<float>(Matrix.m_up.m_x);                                                         //	    Up - X
                Pkt->write<float>(Matrix.m_up.m_y);                                                         //	    Up - Y
                Pkt->write<float>(Matrix.m_up.m_z);                                                         //  	Up - Z
                Pkt->write<float>(Matrix.m_up.m_w);                                                         //  	Up - W
                Pkt->write<float>(Velocity.m_x);                                                            //	    Velocity - X
                Pkt->write<float>(Velocity.m_y);                                                            //	    Velocity - Y
                Pkt->write<float>(Velocity.m_z);                                                            //  	Velocity - Z
                Pkt->write<float>(Velocity.m_w);                                                            //  	Velocity - W
                //
                //
                //  TODO: This will work for now.. But if clients connect in on a different NetPoint then this will not suffice..
                WorldEngine::NetCode::Point->SendPacket(Pkt);
            }
        }
        LastUpdate = CurTime;
    }
}