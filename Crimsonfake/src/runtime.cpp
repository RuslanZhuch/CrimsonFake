#include <Contexts/ApplicationContext.h>
#include <Contexts/BulletsContext.h>
#include <Contexts/CollisionsDataContext.h>
#include <Contexts/CollisionsResultContext.h>
#include <Contexts/EnemiesContext.h>
#include <Contexts/MaterialsContext.h>
#include <Contexts/MouseContext.h>
#include <Contexts/PlayerWorldStateContext.h>
#include <Contexts/RenderCmdsContext.h>

#include <executors/RenderExecutor.h>
#include <executors/PlayerExecutor.h>
#include <executors/WorldExecutor.h>
#include <executors/BulletsExecutor.h>
#include <executors/CollisionsExecutor.h>
#include <executors/EnemiesExecutor.h>
#include <executors/SpawnerExecutor.h>
#include <executors/AssetsExecutor.h>

#include <dod/SharedContext.h>
#include <dod/BufferUtils.h>
#include <chrono>

int main()
{
    Dod::SharedContext::Controller<Game::Context::Application::Data> sApplicationContext;
    Dod::SharedContext::Controller<Game::Context::Materials::Data> materialsContext;
    Dod::SharedContext::Controller<Game::Context::RenderCmds::Data> renderCmdsContext;
    Dod::SharedContext::Controller<Game::Context::Mouse::Data> mouseContext;
    Dod::SharedContext::Controller<Game::Context::Bullets::Data> bulletsToCreateContext;
    Dod::SharedContext::Controller<Game::Context::Enemies::Data> enemiesToSpawnContext;
    Dod::SharedContext::Controller<Game::Context::PlayerWorldState::Data> playerWorldStateContext;
    Dod::SharedContext::Controller<Game::Context::CollisionsResult::Data> collisionsResultContext;
    Dod::SharedContext::Controller<Game::Context::CollisionsData::Data> collisionsDataContext;

    Game::ExecutionBlock::Render render;
    render.loadContext();
    render.cmdsContext = &renderCmdsContext;
    render.materialsContext = &materialsContext;
    render.initiate();
    Game::ExecutionBlock::Player player;
    player.loadContext();
    player.mouseContext = &mouseContext;
    player.initiate();
    Game::ExecutionBlock::World world;
    world.loadContext();
    world.initiate();
    Game::ExecutionBlock::Bullets bullets;
    bullets.loadContext();
    bullets.toCreateContext = &bulletsToCreateContext;
    bullets.collisionsInputContext = &collisionsResultContext;
    bullets.initiate();
    Game::ExecutionBlock::Collisions collisions;
    collisions.loadContext();
    collisions.inputContext = &collisionsDataContext;
    collisions.initiate();
    Game::ExecutionBlock::Enemies enemies;
    enemies.loadContext();
    enemies.toSpawnContext = &enemiesToSpawnContext;
    enemies.playerWorldStateContext = &playerWorldStateContext;
    enemies.collisionsInputContext = &collisionsResultContext;
    enemies.initiate();
    Game::ExecutionBlock::Spawner spawner;
    spawner.loadContext();
    spawner.initiate();
    Game::ExecutionBlock::Assets assets;
    assets.loadContext();
    assets.initiate();

    float deltaTime{};
    while(true)
    {
        const auto start{ std::chrono::high_resolution_clock::now() };


        assets.update(deltaTime);
        bullets.update(deltaTime);
        enemies.update(deltaTime);
        player.update(deltaTime);
        render.update(deltaTime);
        spawner.update(deltaTime);
        world.update(deltaTime);

        Dod::SharedContext::merge(&collisionsDataContext, enemies.collisionsOutputContext);
        Dod::SharedContext::merge(&collisionsDataContext, bullets.collisionsOutputContext);

        collisions.update(deltaTime);

        Dod::SharedContext::flush(&collisionsDataContext);
        Dod::SharedContext::flush(&renderCmdsContext);
        Dod::SharedContext::flush(&enemiesToSpawnContext);
        Dod::SharedContext::flush(&bulletsToCreateContext);
        Dod::SharedContext::flush(&collisionsResultContext);

        Dod::SharedContext::merge(&sApplicationContext, render.applicationContext);
        Dod::SharedContext::merge(&mouseContext, render.mouseContext);
        Dod::SharedContext::merge(&materialsContext, assets.materialsContext);
        Dod::SharedContext::merge(&renderCmdsContext, player.renderCmdsContext);
        Dod::SharedContext::merge(&renderCmdsContext, bullets.renderCmdsContext);
        Dod::SharedContext::merge(&renderCmdsContext, world.renderCmdsContext);
        Dod::SharedContext::merge(&renderCmdsContext, enemies.renderCmdsContext);
        Dod::SharedContext::merge(&bulletsToCreateContext, player.bulletsToCreateContext);
        Dod::SharedContext::merge(&enemiesToSpawnContext, spawner.toSpawnContext);
        Dod::SharedContext::merge(&playerWorldStateContext, player.worldStateContext);
        Dod::SharedContext::merge(&collisionsResultContext, collisions.outputContext);

        render.flushSharedLocalContexts();
        player.flushSharedLocalContexts();
        world.flushSharedLocalContexts();
        bullets.flushSharedLocalContexts();
        collisions.flushSharedLocalContexts();
        enemies.flushSharedLocalContexts();
        spawner.flushSharedLocalContexts();
        assets.flushSharedLocalContexts();

        for (int32_t cmdId{}; cmdId < Dod::BufferUtils::getNumFilledElements(sApplicationContext.context.commands); ++cmdId)
        {
            if (Dod::BufferUtils::get(sApplicationContext.context.commands, 0) == 1)
            {
                return 0;
            }
        }

        const auto end{ std::chrono::high_resolution_clock::now() };
        deltaTime = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) / 1'000'000'000.f;
    }
}
