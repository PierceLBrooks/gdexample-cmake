#include "detourobstacle.h"
#include <CylinderMesh.hpp>
#include <QuadMesh.hpp>
#include <File.hpp>
#include <DetourTileCache.h>

using namespace godot;

#define OBSTACLE_SAVE_VERSION 1

void
DetourObstacle::_register_methods()
{
    register_method("move", &DetourObstacle::move);
    register_method("destroy", &DetourObstacle::destroy);

    register_property<DetourObstacle, Vector3>("position", &DetourObstacle::_position, Vector3(0.0f, 0.0f, 0.0f));
    register_property<DetourObstacle, Vector3>("dimensions", &DetourObstacle::_dimensions, Vector3(0.0f, 0.0f, 0.0f));
}

DetourObstacle::DetourObstacle()
    : _type(OBSTACLE_TYPE_INVALID)
    , _position(Vector3(0.0f, 0.0f, 0.0f))
    , _dimensions(Vector3(0.0f, 0.0f, 0.0f))
    , _destroyed(false)
{

}

DetourObstacle::~DetourObstacle()
{
    if (!_references.empty())
    {
        ERR_PRINT("Destructor of obstacle was called with non-empty references!");
    }
}

void
DetourObstacle::initialize(DetourObstacleType type, const Vector3& position, const Vector3& dimensions, float rotationRad)
{
    _type = type;
    _position = position;
    _dimensions = dimensions;
    _rotationRad = rotationRad;
}

bool
DetourObstacle::save(Ref<File> targetFile)
{
    // Version
    targetFile->store_16(OBSTACLE_SAVE_VERSION);

    // Properties
    targetFile->store_16(_type);
    targetFile->store_var(_position, true);
    targetFile->store_var(_dimensions, true);
    targetFile->store_float(_rotationRad);
    targetFile->store_8(_destroyed);

    return true;
}

bool
DetourObstacle::load(Ref<File> sourceFile)
{
    // Version
    int version = sourceFile->get_16();
    if (version == OBSTACLE_SAVE_VERSION)
    {
        // Properties
        _type = (DetourObstacleType)sourceFile->get_16();
        _position = sourceFile->get_var(true);
        _dimensions = sourceFile->get_var(true);
        _rotationRad = sourceFile->get_float();
        _destroyed = sourceFile->get_8();
    }
    else
    {
        ERR_PRINT(String("Unable to load obstacle. Unknown version: {0}").format(Array::make(version)));
        return false;
    }

    return true;
}

void
DetourObstacle::createDetourObstacle(dtTileCache* cache)
{
    dtObstacleRef ref;
    switch (_type)
    {
    case OBSTACLE_TYPE_CYLINDER:
    {
        float pos[3];
        pos[0] = _position.x;
        pos[1] = _position.y;
        pos[2] = _position.z;
        dtStatus status = cache->addObstacle(pos, _dimensions.x, _dimensions.y, &ref);
        if (dtStatusFailed(status))
        {
            ERR_PRINT("createDetourObstacle: Failed to add cylinder obstacle.");
            return;
        }
        break;
    }

    case OBSTACLE_TYPE_BOX:
    {
        float pos[3];
        pos[0] = _position.x;
        pos[1] = _position.y;
        pos[2] = _position.z;
        float halfExtents[3];
        halfExtents[0] = _dimensions.x * 0.5f;
        halfExtents[1] = _dimensions.y * 0.5f;
        halfExtents[2] = _dimensions.z * 0.5f;
        dtStatus status = cache->addBoxObstacle(pos, halfExtents, _rotationRad, &ref);
        if (dtStatusFailed(status))
        {
            ERR_PRINT("createDetourObstacle: Failed to add box obstacle.");
            return;
        }
        break;
    }

    default:
        ERR_PRINT(String("createDetourObstacle: Invalid obstacle type {0}").format(Array::make(_type)));
        return;
    }

    addReference(ref, cache);
}

void
DetourObstacle::addReference(unsigned int ref, dtTileCache* cache)
{
    _references[cache] = ref;
}

void
DetourObstacle::move(Vector3 position)
{
    // Iterate over all tile caches
    for (auto const& it : _references)
    {
        // Remove the obstacle
        it.first->removeObstacle(it.second);

        // Add the obstacle again at a new position
        _position = position;
        createDetourObstacle(it.first);
    }
}

void
DetourObstacle::destroy()
{
    // Iterate over all tile caches
    for (auto const& it : _references)
    {
        it.first->removeObstacle(it.second);
    }
    _references.clear();
    _destroyed = true;
}
