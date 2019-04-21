/*

    A simple contract implementation of game Bingo on EOS blockchain.

    This contract handles initialization of contract, user login, user logout, gameplay,
        and verification of wining player. Only one user can won in each game
    This contract provides 6 actions:
        login:
            User login to start a game, which a seed string is required for random number generation.
            User is ready to game after this action.
        logout:
            User logout from game. Will reverse "user is ready for game" state caused by login before a game starts.
            Also makes the user lose if game already started.
            A button for manual logout should be provided. 
            Server should also automatically logout player after inactivity.
        entergame:
            Server suppose to call this action after global game_id is incremented, user game_id is smaller than global
            to start game.
            which means we have PLAYER_COUNT number of player ready for a new game.
            A valid board is generated after game start.
        genball: (Description for GAME_SIZE == 5)
            Server suppose to call this action in fixed time intervals until game end (in_game == false).
            First call will generate 5 balls at once, no duplication.
            Second to Sixth call will generate one ball per call, no dupliction with 6-10 balls but can duplicate with first 5.
            Seventh call will cause user to logout, i.e. lose, game end.
        declearwin:
            This action should be triggered by user when the user believe that he/she is winning.
            The contract will then verify if won and change the variables accordingly.
            One player win will cause all other players in the same game lose.
            Calling this action will cause the player to lose if not winning.
        erase:
            Reset game and all tables. Action for testing purpose

    By Yuqing Liu
    CS4284, Virginia Tech 
    Apr 2019
*/

#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/transaction.h>

using namespace std;
using namespace eosio;
#define GAME_SIZE 5         // Will generate GAME_SIZE*GAME_SIZE table and GMAE_SIZE*2 balls. larger size will cause issue for now
#define PLAYER_COUNT 2      // number of players in one game, can be any number

class [[eosio::contract]] bingo : public eosio::contract
{
    private:
        
        struct [[eosio::table]] player  // structure to hold game and player data for each player
        {
            name        username;       // name of player
            uint16_t    win_count;      // times a player won
            uint16_t    lost_count;     // times a player lose
            int8_t      prev_game;      // if a player won/lose the previous game. 1:won, 0:lost, -1:invalid
            
            uint64_t    game_id;        // a number indication which game a player is in. Used to locate other players in same game
            bool        in_game;        // ture: player is in game, false: player is not in game/game ended
            std::string seed;           // user input used for random generation

            //most recent game record
            std::vector<int8_t> board;  // Bingo game table [GAME_SIZE * GAME_SIZE];
            std::vector<int8_t> balls;  // Bingp game balls [GAME_SIZE * 2];
            int8_t iteration;           // indicates ball generation state
            
            uint64_t primary_key() const { return username.value; }     //Primary key cannot be changed, use name
            uint64_t get_secondary_1() const { return game_id; }        //Secondary key can be changed, good for finding players in same game
        };

        typedef eosio::multi_index<"players"_n, player, 
            indexed_by<"bygid"_n, const_mem_fun<player, uint64_t, &player::get_secondary_1>>
        > players_table;
        
        struct [[eosio::table]] record  // Work-around to store contract state
        {
            name        contract;       // name of contract, primary key
            uint64_t    game_id;        // Current ID of game that is awaiting players
            int8_t      cur_group_count;// Counut of awaiting players

            uint64_t primary_key() const { return contract.value; }
        };

        typedef eosio::multi_index<"record"_n, record> record_table;
        
        players_table _players;         // Table for player data
        record_table _record;           // Table for contract state

    public:
        using contract::contract;

        bingo(name username, name code, datastream<const char*> ds ):contract(username, code, ds), _players(username, username.value), _record(_self, _self.value){}
        
        /*
         *  This action will initialize contract (if not initialized) and add user to player table (if not already in there)
         *  User is ready to game after this action.
         *  When a group has enough people, the group number is incremented, whereas the previous group is "detached" for game
         *  @param  seed
         *      User input seed string for random number generation
         */ 
        [[eosio::action]]
        void login(name username, std::string seed)
        {
            require_auth(username);
            auto itr = _record.begin();
            // If no record is in _record, initialize contract
            if (itr == _record.end())
            {
                itr = _record.emplace
                (
                    _self, [&](auto& newrecord)
                    {
                        newrecord.contract = _self;
                        newrecord.cur_group_count = 0;
                        newrecord.game_id = 0;
                    }
                );
                itr = _record.begin();
            }
            auto user_iterator = _players.find(username.value);
            //If player doesn't exist in _player, add to table
            if (user_iterator == _players.end())
            {
                user_iterator = _players.emplace
                (
                    username,  [&](auto& new_user)
                    {
                        new_user.username = username;
                        new_user.win_count = 0;
                        new_user.lost_count = 0;
                        new_user.prev_game = -1;
                        new_user.in_game = false;
                        new_user.seed = seed;
                        new_user.game_id = itr->game_id;
                        new_user.board.reserve(GAME_SIZE * GAME_SIZE);
                        new_user.balls.reserve(GAME_SIZE * 2);
                        int i;
                        for (i = 0; i < GAME_SIZE*GAME_SIZE; i++)
                            new_user.board.push_back(-1);
                        for (i = 0; i < GAME_SIZE*2; i++)
                            new_user.balls.push_back(-1);
                    }
                );
            }
            // Modify table entry such that player is waiting for game
            else
            {
                _players.modify
                (
                    user_iterator, username, [&](auto& player)
                    {
                        player.seed = seed;
                        player.in_game = false;
                        player.game_id = itr->game_id;
                    }
                );
            }
            // Modify table entry to incremenet group count
            //if we have enough people, increment game id to "detach" the group
            _record.modify
            (
                itr, _self, [&](auto& r)
                {
                    r.cur_group_count = r.cur_group_count + 1;
                    print("cur group has ", r.cur_group_count, ", game id is",  r.game_id);
                    if (r.cur_group_count >= PLAYER_COUNT)
                    {
                        r.game_id = r.game_id + 1;
                        r.cur_group_count = 0;
                        print("Now a group is ready to game");
                    }
                }
            );
        }

        /*
         *  This action will logout user.
         *  If user is waiting for game to start, decrement group count
         *  If user is already in game, the user lose the game
         *  User is not in game after this action
         */ 
        [[eosio::action]]
        void logout(name username)
        {
            require_auth(username);
            auto itr = _record.begin();
            eosio_assert(itr != _record.end(), "Player does not exist");
            auto iterator = _players.find(username.value);
            eosio_assert(iterator != _players.end(), "Player does not exist");
            _players.modify
            (
                iterator, username, [&](auto& player)
                {
                    player.iteration = -3;
                    //Make user lose
                    if (player.in_game)
                    {
                        player.lost_count = player.lost_count + 1;
                        player.prev_game = 0;
                        player.iteration = -2;
                    }
                    //decrement group count
                    else if (player.game_id == itr->game_id && itr->cur_group_count != 0)
                        _record.modify
                        (
                            itr, _self, [&](auto& r)
                            {
                                r.cur_group_count =  r.cur_group_count - 1;
                            }
                        );
                    player.in_game = false;
                }
            );
        }

        /*
         *  After global game id is incremented, this actions should be called to start a game
         *  A game board will be generated using the transaction and the user input seed
         *  After this action call the in_game variable is true, plahyer is in game.
         *  A gen_ball call should follow this to generate first 5 balls.
         */
        [[eosio::action]]
        void entergame(name username)
        {
            require_auth(username);
            auto ritr = _record.begin();
            eosio_assert(ritr != _record.end(), "Player does not exist");
            auto iterator = _players.find(username.value);
            eosio_assert(iterator != _players.end(), "Player does not exist");
            eosio_assert(iterator->in_game != true, "Player already in game!");
            eosio_assert(iterator->game_id != ritr->game_id, "Group not ready");
            _players.modify
            (
                iterator, username, [&](auto& player)
                {
                    // User in game
                    player.in_game = true;
                    player.iteration = 0;
                    // Random using transaction info, user seed, and hash
                    auto size = transaction_size();
                    char* buf = new char[size + strlen(player.seed.c_str())];
                    uint32_t read = read_transaction(buf, size);
                    strcpy(buf + read, player.seed.c_str());
                    capi_checksum256 hash;
                    sha256(buf, read + strlen(player.seed.c_str()), &hash);
                    delete [] buf;
                    //Generate the board
                    int8_t* t = new int8_t[GAME_SIZE * GAME_SIZE];
                    int8_t i, k = 0;
                    for (i = 0; i < GAME_SIZE * GAME_SIZE; i++)
                        t[i] = i;
                    for (i = 0; i < GAME_SIZE * GAME_SIZE; i++)
                    {
                        int8_t j = ((uint8_t)hash.hash[i]) % (GAME_SIZE * GAME_SIZE - i);
                        while (1)
                        {
                            if (t[k] >= 0)
                            {
                                j--;
                                if (j == -1)
                                    break;
                            }
                            k++;
                            k = k % (GAME_SIZE * GAME_SIZE);
                        }
                        player.board[i] = t[k];
                        t[k] = -1;
                    }
                    delete [] t;
                    // clear ball
                    for (i = 0; i < GAME_SIZE * 2; i++)
                        player.balls[i] = -1;
                }
            );
        }

        /*
         *  First call will generate first GAME_SIZE balls, no duplicates
         *  Next GAME_SIZE calls generates one ball each, no duplicates with later balls but can duplicate balls with first call
         *  One extra call will cause the user to logout
         */
        [[eosio::action]]
        void genball(name username)
        {
            require_auth(username);
            auto ritr = _record.begin();
            eosio_assert(ritr != _record.end(), "Player does not exist");
            auto iterator = _players.find(username.value);
            eosio_assert(iterator != _players.end(), "Player does not exist");
            eosio_assert(iterator->in_game, "Player not in game!");
            //No ball to generate, logout
            if (iterator->iteration > GAME_SIZE)
            {
                logout(username);
                return;
            }
            _players.modify
            (
                iterator, username, [&](auto& player)
                {
                    bool end_flag = true;
                    auto ptable = _players.get_index<"bygid"_n>();
                    auto itr = ptable.lower_bound(player.game_id);
                    std::string all_seed = "";
                    bool s_flag = true;
                    while (itr != ptable.end())
                    {
                        if (itr->game_id != player.game_id)
                            break;
                        if (itr->iteration == -1)               //someone won
                        {
                            end_flag = false;
                            break;
                        }
                        if (itr->iteration > player.iteration)  //sync balls with other players
                        {
                            s_flag = false;
                            int i;
                            for (i = 0; itr->balls[i] >= 0 && i < GAME_SIZE * 2; i++)
                                player.balls[i] = itr->balls[i];
                            player.iteration = itr->iteration;
                            break;
                        }
                        if (itr->iteration != -3)
                            all_seed = all_seed + itr->seed;
                        itr++;
                    }
                    if (end_flag)   //If no one end yet
                    {
                        if (s_flag) //first 5 generation
                        {
                            capi_checksum256 h;
                            int8_t* t = new int8_t[GAME_SIZE * GAME_SIZE];
                            int i;
                            for (i = 0; i < GAME_SIZE * GAME_SIZE; i++)
                                t[i] = i;
                            if (player.iteration == 0) //first 5 balls
                            {
                                sha256(all_seed.c_str(), strlen(all_seed.c_str()), &h);
                                int k = 0;
                                for (i = 0; i < GAME_SIZE; i++)
                                {
                                    uint32_t j = h.hash[i * 4] << 24 | h.hash[i * 4 + 1] << 16 | h.hash[i * 4 + 2] << 8 | h.hash[i * 4 + 3];
                                    j = j % (GAME_SIZE * GAME_SIZE - i);
                                    while (1)
                                    {
                                        if (t[k] >= 0)
                                        {
                                            j--;
                                            if (j == -1)
                                                break;
                                        }
                                        k++;
                                        k = k % (GAME_SIZE * GAME_SIZE);
                                    }
                                    player.balls[i] = t[k];
                                    t[k] = -1;
                                }
                            }
                            else    // later one-each-call ball generation
                            {
                                auto size = transaction_size();
                                char* buf = new char[size + strlen(all_seed.c_str())];
                                uint32_t read = read_transaction(buf, size);
                                strcpy(buf + read, all_seed.c_str());
                                sha256(buf, read + strlen(all_seed.c_str()), &h);
                                delete [] buf;
                                int8_t k = 0;
                                for (i = 0; i < GAME_SIZE; i++)
                                    if (player.balls[GAME_SIZE + i] >= 0)
                                        t[player.balls[GAME_SIZE + i]] = -1;
                                    else
                                        break;
                                uint32_t j = h.hash[i * 4] << 24 | h.hash[i * 4 + 1] << 16 | h.hash[i * 4 + 2] << 8 | h.hash[i * 4 + 3];
                                j = j % (GAME_SIZE * GAME_SIZE - i - 1);
                                while (1)
                                {
                                    if (t[k] >= 0)
                                    {
                                        j--;
                                        if (j == -1)
                                            break;
                                    }
                                    k++;
                                    k = k % (GAME_SIZE * GAME_SIZE);
                                }
                                player.balls[GAME_SIZE + i - 1] = t[k];
                                eosio_assert(i + 1 == player.iteration, "We have sanity problem?");
                            }
                            delete [] t;
                            player.iteration = player.iteration + 1;
                        }
                    }
                    else    //Someone won, current user lost
                    {
                        player.iteration = -2;
                        player.lost_count = player.lost_count + 1;
                        player.prev_game = 0;
                        player.in_game = false;
                    }
                }
            );
        }
        
        /*
         *  This action should only be called if one user believe he/she is winning
         *  This action will verify the winning state and increment win counter
         *  Or the user loses immediatly
         */
        [[eosio::action]]
        void declearwin(name username)
        {
            require_auth(username);
            auto ritr = _record.begin();
            eosio_assert(ritr != _record.end(), "Player does not exist");
            auto iterator = _players.find(username.value);
            eosio_assert(iterator->in_game, "Player not in game!");
            _players.modify
            (
                iterator, username, [&](auto& player)
                {
                    bool end_flag = true;
                    auto ptable = _players.get_index<"bygid"_n>();
                    auto itr = ptable.lower_bound(player.game_id);
                    while (itr != ptable.end())
                    {
                        if (itr->game_id != player.game_id)
                            break;
                        if (itr->iteration == -1)//someone already won
                        {
                            end_flag = false;
                            break;
                        }
                        itr++;
                    }
                    if (end_flag)
                    {
                        //check if winning
                        int8_t* t = new int8_t[GAME_SIZE * GAME_SIZE];
                        int8_t i, j, r, c, d, e, w = 0;
                        for (i = 0; i < GAME_SIZE * GAME_SIZE; i++)
                            t[i] = 0;
                        t[12] = 1; //center bonus
                        for (i = 0; i < GAME_SIZE * 2; i++)
                            for (j = 0; j < GAME_SIZE * GAME_SIZE; j++)
                                if (player.balls[i] == player.board[j])
                                    t[j] = 1;
                        d = e = 1;
                        for (i = 0; i < GAME_SIZE; i++)
                        {
                            r = c = 1;
                            d &= t[i * GAME_SIZE + i];
                            e &= t[(i + 1) * GAME_SIZE - i - 1];
                            for (j = 0; j < GAME_SIZE; j++)
                            {
                                r &= t[i * GAME_SIZE + j];
                                c &= t[j * GAME_SIZE + i];
                            }
                            w |= r | c; // row & column
                        }
                        w |= d | e; //diagnal
                        delete [] t;
                        if (w == 1)
                        {
                            player.iteration = -1;
                            player.win_count = player.win_count + 1;
                            player.prev_game = 1;
                            player.in_game = false;
                        }
                        else
                        {
                            player.iteration = -2;
                            player.lost_count = player.lost_count + 1;
                            player.prev_game = 0;
                            player.in_game = false;
                        }
                    }
                    else    //Someone else won, current user lose
                    {
                        player.iteration = -2;
                        player.lost_count = player.lost_count + 1;
                        player.prev_game = 0;
                        player.in_game = false;
                    }
                }
            );
        }

        /*
         *  Reset game by erasing player table
         *  and reset contract record
         */
        [[eosio::action]]
        void erase( name self )
        {
            require_auth( _self );
            auto ritr = _record.begin();
            eosio_assert(ritr != _record.end(), "Table does not exist");
            auto itr = _players.begin();
            while (itr != _players.end())
            {
                itr=_players.erase(itr);
            }
            _record.modify
            (
                ritr, _self, [&](auto& r)
                {
                    r.game_id = 0;
                    r.cur_group_count = 0;
                }
            );
        }
};  

EOSIO_DISPATCH( bingo, (login)(logout)(entergame)(genball)(declearwin)(erase))