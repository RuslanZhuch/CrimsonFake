#include <Contexts/ApplicationContext.h>
#include <Contexts/MaterialsContext.h>
#include <Contexts/MouseContext.h>
#include <Contexts/RenderCmdsContext.h>

#include <executors/RenderExecutor.h>
#include <executors/PlayerExecutor.h>
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

    Game::ExecutionBlock::Render render;
    render.loadContext();
    render.cmdsContext = &renderCmdsContext;
    render.materialsContext = &materialsContext;
    render.initiate();
    Game::ExecutionBlock::Player player;
    player.loadContext();
    player.mouseContext = &mouseContext;
    player.initiate();
    Game::ExecutionBlock::Assets assets;
    assets.loadContext();
    assets.initiate();

    float deltaTime{};
    while(true)
    {
        const auto start{ std::chrono::high_resolution_clock::now() };

        render.update(deltaTime);
        player.update(deltaTime);
        assets.update(deltaTime);

        Dod::SharedContext::flush(&renderCmdsContext);

        Dod::SharedContext::merge(&sApplicationContext, render.applicationContext);
        Dod::SharedContext::merge(&mouseContext, render.mouseContext);
        Dod::SharedContext::merge(&materialsContext, assets.materialsContext);
        Dod::SharedContext::merge(&renderCmdsContext, player.renderCmdsContext);

        render.flushSharedLocalContexts();
        player.flushSharedLocalContexts();
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
