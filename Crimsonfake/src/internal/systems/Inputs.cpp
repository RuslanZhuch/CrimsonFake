#include "Inputs.h"

#include <dod/ConditionTable.h>
#include <dod/BufferUtils.h>

#include <SFML/Window/Keyboard.hpp>
#include <array>

//Game::Inputs::MouseCoord gatherMouseCoord() noexcept
//{
//
//    const auto mousePos{ sf::Mouse::getPosition() };
//    return { mousePos.x, mousePos.y };
//
//}

uint32_t Game::Inputs::gatherInputs() noexcept
{

    constexpr uint32_t moveLeftControlBit{ uint32_t(1) << 0 };
    constexpr uint32_t moveRightControlBit{ uint32_t(1) << 1 };
    constexpr uint32_t moveBackwardControlBit{ uint32_t(1) << 2 };
    constexpr uint32_t moveForwardControlBit{ uint32_t(1) << 3 };
    constexpr uint32_t fireControlBit{ uint32_t(1) << 4 };

    uint32_t inputBits{};

    inputBits |= (moveLeftControlBit)*sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    inputBits |= (moveRightControlBit)*sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    inputBits |= (moveBackwardControlBit)*sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    inputBits |= (moveForwardControlBit)*sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    inputBits |= (fireControlBit)*sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

    return inputBits;

}

template <size_t numOfCols>
using condInput_t = std::array<Dod::CondTable::TriState, numOfCols>;
template <size_t numOfRows, size_t numOfCols>
using condTableSrc_t = std::array<condInput_t<numOfCols>, numOfRows>;

int32_t Game::Inputs::computeFireComponent(uint32_t currentInputs, uint32_t prevInputs) noexcept
{

    // currPress, prevPress
    const condTableSrc_t<3, 2> tableSrc{ {
        { Dod::CondTable::TriState::TRUE, Dod::CondTable::TriState::FALSE },
        { Dod::CondTable::TriState::TRUE, Dod::CondTable::TriState::TRUE },
        { Dod::CondTable::TriState::FALSE, Dod::CondTable::TriState::TRUE },
    } };

    std::array<uint32_t, tableSrc.size() + 1> xOrMasksMem;
    std::array<uint32_t, tableSrc.size() + 1> ignoreMem;

    const auto table{ Dod::CondTable::generate(tableSrc, xOrMasksMem, ignoreMem) };

    const uint32_t inputs{
        (uint32_t((currentInputs & 16) != 0) << 0) |
        (uint32_t((prevInputs & 16) != 0) << 1)
    };

    std::array<int32_t, 16> qValuesMem;
    Dod::DBBuffer<int32_t> qValues;
    Dod::BufferUtils::initFromArray(qValues, qValuesMem);

    Dod::CondTable::populateQuery(qValues, inputs, table);

    const auto transformOutputs{ std::to_array<int32_t>({
        1,
        1,
        0,
    }) };

    int32_t fireComponent{};
    Dod::CondTable::applyTransform<int32_t, int32_t>(fireComponent, transformOutputs, Dod::BufferUtils::createImFromBuffer(qValues));

    return fireComponent;

}

float computePartialMoveComponent(uint32_t currentInputs, uint32_t prevInputs, float prevMovement) noexcept
{

    // currLeft, prevLeft, currRight, prevRight
    const condTableSrc_t<4, 4> tableSrc{ {
        { Dod::CondTable::TriState::TRUE, Dod::CondTable::TriState::FALSE, Dod::CondTable::TriState::SKIP, Dod::CondTable::TriState::SKIP },
        { Dod::CondTable::TriState::SKIP, Dod::CondTable::TriState::SKIP, Dod::CondTable::TriState::TRUE, Dod::CondTable::TriState::FALSE },
        { Dod::CondTable::TriState::FALSE, Dod::CondTable::TriState::TRUE, Dod::CondTable::TriState::FALSE, Dod::CondTable::TriState::SKIP },
        { Dod::CondTable::TriState::FALSE, Dod::CondTable::TriState::SKIP, Dod::CondTable::TriState::FALSE, Dod::CondTable::TriState::TRUE },
    } };

    std::array<uint32_t, tableSrc.size() + 1> xOrMasksMem;
    std::array<uint32_t, tableSrc.size() + 1> ignoreMem;

    const auto table{ Dod::CondTable::generate(tableSrc, xOrMasksMem, ignoreMem) };

    const uint32_t inputs{
        (uint32_t((currentInputs & 1) != 0) << 0) |
        (uint32_t((prevInputs & 1) != 0) << 1) |
        (uint32_t((currentInputs & 2) != 0) << 2) |
        (uint32_t((prevInputs & 2) != 0) << 3)
    };

    std::array<int32_t, 16> qValuesMem;
    Dod::DBBuffer<int32_t> qValues;
    Dod::BufferUtils::initFromArray(qValues, qValuesMem);

    Dod::CondTable::populateQuery(qValues, inputs, table);

    const auto transformOutputs{ std::to_array<float>({
        -1.f,
        1.f,
        0.f,
        0.f,
    }) };

    float moveComponent{ prevMovement };
    Dod::CondTable::applyTransform<int32_t, float>(moveComponent, transformOutputs, Dod::BufferUtils::createImFromBuffer(qValues));

    return moveComponent;

}

Game::Inputs::MoveState Game::Inputs::computeMoveComponent(uint32_t currentInputs, uint32_t prevInputs, MoveState prevMovement) noexcept
{

    const auto rightComponent{ computePartialMoveComponent(currentInputs, prevInputs, prevMovement.right) };
    const auto forwardComponent{ computePartialMoveComponent(currentInputs >> 2, prevInputs >> 2, prevMovement.forward) };

    return { rightComponent, forwardComponent };

}

int32_t Game::Inputs::computeWeaponSwitchComponent() noexcept
{
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
        return 0;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
        return 1;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
        return 2;

    return -1;

}
