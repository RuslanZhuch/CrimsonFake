#include <Contexts/ApplicationContext.h>
#include <Contexts/BulletsContext.h>
#include <Contexts/CollisionsDataContext.h>
#include <Contexts/CollisionsResultContext.h>
#include <Contexts/DecalsCmdsContext.h>
#include <Contexts/EnemiesContext.h>
#include <Contexts/ItemsCmdsContext.h>
#include <Contexts/MouseContext.h>
#include <Contexts/PlayerWorldStateContext.h>
#include <Contexts/RenderCmdsContext.h>
#include <Contexts/WeaponCmdsContext.h>

#include <executors/RenderExecutor.h>
#include <executors/PlayerExecutor.h>
#include <executors/WorldExecutor.h>
#include <executors/DecalsExecutor.h>
#include <executors/BulletsExecutor.h>
#include <executors/CollisionsExecutor.h>
#include <executors/EnemiesExecutor.h>
#include <executors/SpawnerExecutor.h>
#include <executors/WeaponsExecutor.h>
#include <executors/ItemsExecutor.h>
#include <executors/AssetsExecutor.h>

#include <dod/SharedContext.h>
#include <dod/BufferUtils.h>
#include <chrono>

int main()
{
    Dod::SharedContext::Controller<Game::Context::Application::Data> sApplicationContext;
    Dod::SharedContext::Controller<Game::Context::RenderCmds::Data> renderCmdsContext;
    Dod::SharedContext::Controller<Game::Context::Mouse::Data> mouseContext;
    Dod::SharedContext::Controller<Game::Context::Bullets::Data> bulletsToCreateContext;
    Dod::SharedContext::Controller<Game::Context::Enemies::Data> enemiesToSpawnContext;
    Dod::SharedContext::Controller<Game::Context::PlayerWorldState::Data> playerWorldStateContext;
    Dod::SharedContext::Controller<Game::Context::CollisionsResult::Data> collisionsResultContext;
    Dod::SharedContext::Controller<Game::Context::WeaponCmds::Data> weaponCmdsContext;
    Dod::SharedContext::Controller<Game::Context::ItemsCmds::Data> itemsCmdsContext;
    Dod::SharedContext::Controller<Game::Context::DecalsCmds::Data> decalsCmdsContext;
    Dod::SharedContext::Controller<Game::Context::CollisionsData::Data> collisionsDataContext;

    Game::ExecutionBlock::Render render;
    render.loadContext();
    render.cmdsContext = &renderCmdsContext;
    render.initiate();
    Game::ExecutionBlock::Player player;
    player.loadContext();
    player.mouseContext = &mouseContext;
    player.initiate();
    Game::ExecutionBlock::World world;
    world.loadContext();
    world.initiate();
    Game::ExecutionBlock::Decals decals;
    decals.loadContext();
    decals.playerWorldStateContext = &playerWorldStateContext;
    decals.commandsContext = &decalsCmdsContext;
    decals.initiate();
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
    spawner.playerWorldStateContext = &playerWorldStateContext;
    spawner.initiate();
    Game::ExecutionBlock::Weapons weapons;
    weapons.loadContext();
    weapons.commandsContext = &weaponCmdsContext;
    weapons.initiate();
    Game::ExecutionBlock::Items items;
    items.loadContext();
    items.playerWorldStateContext = &playerWorldStateContext;
    items.commandsContext = &itemsCmdsContext;
    items.initiate();
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
        weapons.update(deltaTime);
        world.update(deltaTime);
        items.update(deltaTime);
        decals.update(deltaTime);

        Dod::SharedContext::merge(&collisionsDataContext, enemies.collisionsOutputContext);
        Dod::SharedContext::merge(&collisionsDataContext, bullets.collisionsOutputContext);

        collisions.update(deltaTime);

        Dod::SharedContext::flush(&collisionsDataContext);
        Dod::SharedContext::flush(&renderCmdsContext);
        Dod::SharedContext::flush(&enemiesToSpawnContext);
        Dod::SharedContext::flush(&bulletsToCreateContext);
        Dod::SharedContext::flush(&collisionsResultContext);
        Dod::SharedContext::flush(&weaponCmdsContext);
        Dod::SharedContext::flush(&itemsCmdsContext);
        Dod::SharedContext::flush(&decalsCmdsContext);

        Dod::SharedContext::merge(&sApplicationContext, render.applicationContext);
        Dod::SharedContext::merge(&mouseContext, render.mouseContext);
        Dod::SharedContext::merge(&renderCmdsContext, assets.renderCmdsContext);
        Dod::SharedContext::merge(&renderCmdsContext, player.renderCmdsContext);
        Dod::SharedContext::merge(&renderCmdsContext, bullets.renderCmdsContext);
        Dod::SharedContext::merge(&renderCmdsContext, world.renderCmdsContext);
        Dod::SharedContext::merge(&renderCmdsContext, enemies.renderCmdsContext);
        Dod::SharedContext::merge(&renderCmdsContext, decals.renderCmdsContext);
        Dod::SharedContext::merge(&renderCmdsContext, items.renderCmdsContext);
        Dod::SharedContext::merge(&enemiesToSpawnContext, spawner.toSpawnContext);
        Dod::SharedContext::merge(&itemsCmdsContext, spawner.itemsCmdsContext);
        Dod::SharedContext::merge(&playerWorldStateContext, player.worldStateContext);
        Dod::SharedContext::merge(&collisionsResultContext, collisions.outputContext);
        Dod::SharedContext::merge(&weaponCmdsContext, player.weaponCmdsContext);
        Dod::SharedContext::merge(&decalsCmdsContext, player.decalsCmdsContext);
        Dod::SharedContext::merge(&decalsCmdsContext, enemies.decalsCmdsContext);
        Dod::SharedContext::merge(&bulletsToCreateContext, weapons.bulletsToCreateContext);

        render.flushSharedLocalContexts();
        player.flushSharedLocalContexts();
        world.flushSharedLocalContexts();
        decals.flushSharedLocalContexts();
        bullets.flushSharedLocalContexts();
        collisions.flushSharedLocalContexts();
        enemies.flushSharedLocalContexts();
        spawner.flushSharedLocalContexts();
        weapons.flushSharedLocalContexts();
        items.flushSharedLocalContexts();
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
