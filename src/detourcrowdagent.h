#ifndef DETOURCROWDAGENT_H
#define DETOURCROWDAGENT_H

#include <Godot.hpp>
#include <vector>
#include <map>
#include <atomic>
#include <chrono>

class dtCrowdAgent;
class dtCrowd;
class dtNavMeshQuery;
class dtQueryFilter;
class DetourInputGeometry;

namespace godot
{
    class DetourNavigationMesh;
    class File;

    /**
     * @brief Parameters to initialize a DetourCrowdAgent.
     */
    struct DetourCrowdAgentParameters : public Reference
    {
        GODOT_CLASS(DetourCrowdAgentParameters, Reference)

    public:
        /**
         * @brief Called when .new() is called in gdscript
         */
        void _init() {}

        static void _register_methods();

        Vector3 position;

        // These two parameters will determine into which navigation mesh & crowd this agent will be put.
        // Make sure your DetourNavigationMesh supports the radius & height.
        float   radius;
        float   height;

        float   maxAcceleration;
        float   maxSpeed;

        // The filter to use
        String filterName;

        // Check more in-depth descriptions of the optimizations here:
        // http://digestingduck.blogspot.com/2010/11/path-corridor-optimizations.html
        bool    anticipateTurns;    // If this agent should anticipate turns and move accordingly.
        bool    optimizeVisibility; // Optimize walked path based on visibility. Strongly recommended.
        bool    optimizeTopology;   // If shorter paths should be attempted under certain circumstances. Also recommended.

        bool    avoidObstacles;     // If this agent should try to avoid obstacles (dynamic obstacles).
        bool    avoidOtherAgents;   // If this agent should avoid other agents.
        int     obstacleAvoidance;  // How much this agent should avoid obstacles. 0 - 3, with 0 being low and 3 high avoidance.
        float   separationWeight;   // How strongly the other agents should try to avoid this agent (if they have avoidOtherAgents set).
    };

    // Different states that an agent can be in
    enum DetourCrowdAgentState
    {
        AGENT_STATE_INVALID = -1,
        AGENT_STATE_IDLE,
        AGENT_STATE_GOING_TO_TARGET,
        NUM_AGENT_STATES
    };

    /**
     * @brief A single agent in a crowd.
     */
    class DetourCrowdAgent : public Reference
    {
        GODOT_CLASS(DetourCrowdAgent, Reference)

    public:
        static void _register_methods();

        /**
         * @brief Constructor.
         */
        DetourCrowdAgent();

        /**
         * @brief Destructor.
         */
        ~DetourCrowdAgent();

        /**
         * @brief Called when .new() is called in gdscript
         */
        void _init() {}

        /**
         * @brief Will save this agent's current state to the passed file.
         * @param targetFile The file to append data to.
         * @return True if everything worked out, false otherwise.
         */
        bool save(Ref<File> targetFile);

        /**
         * @brief Loads the agent from the file.
         * @param sourceFile The file to read data from.
         * @return True if everything worked out, false otherwise.
         */
        bool load(Ref<File> sourceFile);

        /**
         * @brief Loads agent parameters from file
         * @param sourceFile The file to read data from.
         * @return True if everything worked out, false otherwise.
         */
        bool loadParameterValues(Ref<DetourCrowdAgentParameters> params, Ref<File> sourceFile);

        /**
         * @brief Sets this agent's main crowd agent.
         */
        void setMainAgent(dtCrowdAgent* crowdAgent, dtCrowd* crowd, int index, dtNavMeshQuery* query, DetourInputGeometry* geom, int crowdIndex);

        /**
         * @brief Sets the filter this agent will use.
         */
        void setFilter(int filterIndex);

        /**
         * @return Return the index of the filter.
         */
        int getFilterIndex();

        /**
         * @return Return the index of the crowd (= index of navmesh).
         */
        int getCrowdIndex();

        /**
         * @return True if the agent is currently moving.
         */
        bool isMoving();

        /**
         * @return The target position for this agent (doesn't necessarily mean that it is currently moving).
         */
        Vector3 getTargetPosition();

        /**
         * @brief Adds the passed agent as a shadow agent that will be updated with the main agent's values regularly.
         */
        void addShadowAgent(dtCrowdAgent* crowdAgent);

        /**
         * @brief The agent will start moving as close as possible towards the passed position.
         */
        void moveTowards(Vector3 position);

        /**
         * @brief Will fill the passed vector with the current movement target, THEN RESET IT.
         * @return True if there was a new target.
         */
        void applyNewTarget();

        /**
         * @brief Stops moving entirely.
         */
        void stop();

        /**
         * @brief Returns a prediction of the movement, based on the passed position and the last updated agent position and velocity.
         * @param currentPos    The position of the external entity.
         * @param currentDir    The current direction of the external entity.
         * @param positionTimestamp    The timestamp of the passed position in ticks milliseconds (best to use OS.get_ticks_msec()).
         * @param maxTurningRad The maximum amount to turn the direction in this call.
         * @return A dictionary with a "position" and a "direction" entry.
         */
        Dictionary getPredictedMovement(Vector3 currentPos, Vector3 currentDir, int64_t positionTicksTimestamp, float maxTurningRad);

        /**
         * @brief Will update the shadows with the current values from the primary crowd.
         */
        void update(float secondsSinceLastTick);

        /**
         * @brief Removes the agent from all crowds it is in and frees all associated memory.
         */
        void destroy();

    private:
        dtCrowdAgent*                   _agent;
        dtCrowd*                        _crowd;
        int                             _agentIndex;
        int                             _crowdIndex;
        dtNavMeshQuery*                 _query;
        dtQueryFilter*                  _filter;
        int                             _filterIndex;
        DetourInputGeometry*            _inputGeom;
        std::vector<dtCrowdAgent*>      _shadows;

        Vector3                 _position;
        Vector3                 _velocity;
        Vector3                 _targetPosition;
        std::atomic_bool        _hasNewTarget;
        DetourCrowdAgentState   _state;

        bool    _isMoving;
        float   _lastDistanceToTarget;
        float   _distanceTotal;
        float   _distanceTime;
        Vector3 _lastPosition;
        float   _movementTime;
        float   _movementOverTime;

        std::chrono::system_clock::time_point lastUpdateTime;
    };

    // INLINE FUNCTIONS
    inline int
    DetourCrowdAgent::getFilterIndex()
    {
        return _filterIndex;
    }

    inline int
    DetourCrowdAgent::getCrowdIndex()
    {
        return _crowdIndex;
    }

    inline bool
    DetourCrowdAgent::isMoving()
    {
        return _isMoving;
    }

    inline Vector3
    DetourCrowdAgent::getTargetPosition()
    {
        return _targetPosition;
    }
}

#endif // DETOURCROWDAGENT_H
