#pragma once

#include "MainLogic/fenParser.hpp"
#include "MainLogic/PlayerVPlayer.hpp"
#include "MainLogic/PlayerVComputer.hpp"
#include "MainLogic/realization.hpp"
#include "Graphic/graphic.hpp"
#include "Board/board.hpp"
#include "MainLogic/movegen.hpp"

namespace chess
{

    class ChessGame
    {
    private:
        Realization &real;

    public:
        ChessGame(Realization &realizazion) : real(realizazion) {}

        void start()
        {
            real.start_game();
        }
    };

}