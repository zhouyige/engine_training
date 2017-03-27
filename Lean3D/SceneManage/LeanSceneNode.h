#pragma once
#include "LeanUtil.h"

namespace Lean3D
{

	struct SceneNodeTpl
	{
		SceneNodeType                  type;
		std::string                    name;
		Vec3                          trans, rot, scale;
		std::string                    attachmentString;
		std::vector< SceneNodeTpl * >  children;

		SceneNodeTpl(SceneNodeType type, const std::string &name) :
			type(type), name(name), scale(Vec3(1, 1, 1))
		{
		}

		virtual ~SceneNodeTpl()
		{
			for (uint32 i = 0; i < children.size(); ++i) delete children[i];
		}
	};

	class SceneNode
	{
	public:
		SceneNode(const SceneNodeTpl &tpl);
		virtual ~SceneNode();
		void getTransform(Vec3 &trans, Vec3 &rot, Vec3 &scale);	// Not virtual for performance
		void setTransform(Vec3 trans, Vec3 rot, Vec3 scale);	// Not virtual for performance
		void setTransform(const Matrix4 &mat);
		void getTransMatrices(const float **relMat, const float **absMat) const;

		int getFlags() { return _flags; }
		void setFlags(int flags, bool recursive);

		//virtual int getParamI(int param);
		//virtual void setParamI(int param, int value);
		//virtual float getParamF(int param, int compIdx);
		//virtual void setParamF(int param, int compIdx, float value);
		//virtual const char *getParamStr(int param);
		//virtual void setParamStr(int param, const char* value);

		virtual uint32 calcLodLevel(const Vec3 &viewPoint);

		virtual bool canAttach(SceneNode &parent);
		void markDirty();
		void updateTree();
		virtual bool checkIntersection(const Vec3 &rayOrig, const Vec3 &rayDir, Vec3 &intsPos) const;

		virtual void setCustomInstData(float *data, uint32 count) {}

		SceneNodeType getType() { return _type; };
		NodeHandle getHandle() { return _handle; }
		SceneNode *getParent() { return _parent; }
		const std::string &getName() { return _name; }
		std::vector< SceneNode * > &getChildren() { return _children; }
		Matrix4 &getRelTrans() { return _relTrans; }
		Matrix4 &getAbsTrans() { return _absTrans; }
		BoundingBox &getBBox() { return _bBox; }
		const std::string &getAttachmentString() { return _attachment; }
		void setAttachmentString(const char* attachmentData) { _attachment = attachmentData; }
		bool checkTransformFlag(bool reset)
		{
			bool b = _transformed; if (reset) _transformed = false; return b;
		}

	protected:
		void markChildrenDirty();

		virtual void onPostUpdate() {}  // Called after absolute transformation has been updated
		virtual void onFinishedUpdate() {}  // Called after children have been updated
		virtual void onAttach(SceneNode &parentNode) {}  // Called when node is attached to parent
		virtual void onDetach(SceneNode &parentNode) {}  // Called when node is detached from parent
	protected:
		Matrix4                     _relTrans, _absTrans;   
		SceneNode                   *_parent;  
		SceneNodeType               _type;
		NodeHandle                  _handle;
		uint32                      _sgHandle;  // Spatial graph handle
		uint32						_flags;
		float                       _sortKey;
		bool                        _dirty;		// Does the node need to be updated?
		bool                        _transformed;
		bool                        _renderable;

		BoundingBox                 _bBox;		

		std::vector< SceneNode * >  _children; 
		std::string                 _name;
		std::string                 _attachment;

		friend class SceneManager;
		friend class SpatialGraph;
		friend class Renderer;
	};
}


