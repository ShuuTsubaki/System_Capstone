#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/transaction.h>

using namespace std;
using namespace eosio;
#define GAME_SIZE 5 //larger size will cause issue
#define PLAYER_COUNT 2

class [[eosio::contract]] bingo : public eosio::contract
{
    private:
        
        struct [[eosio::table]] player
        {
            name        username;
            uint16_t    win_count;
            uint16_t    lost_count;
            int8_t      prev_game;
            
            uint64_t    game_id;
            bool        in_game;
            std::string seed;

            //most recent game record
            std::vector<int8_t> board;//[GAME_SIZE * GAME_SIZE];
            std::vector<int8_t> balls;//[GAME_SIZE * 2];
            int8_t iteration;
            capi_checksum256 block_seed;
            
            uint64_t primary_key() const { return username.value; }
            uint64_t get_secondary_1() const { return game_id; }
        };

        typedef eosio::multi_index<"players"_n, player, 
            indexed_by<"bygid"_n, const_mem_fun<player, uint64_t, &player::get_secondary_1>>
        > players_table;
        
        struct [[eosio::table]] record
        {
            name        contract;
            uint64_t    game_id;
            int8_t      cur_group_count;

            uint64_t primary_key() const { return contract.value; }
        };

        typedef eosio::multi_index<"record"_n, record> record_table;
        
        players_table _players;
        record_table _record;

    public:
        using contract::contract;

        bingo(name username, name code, datastream<const char*> ds ):contract(username, code, ds), _players(username, username.value), _record(_self, _self.value){}
        
        //call this ONCE and wait until in_game become true
        [[eosio::action]]
        void login(name username, std::string seed)
        {
            require_auth(username);
            auto itr = _record.begin();
            if (itr == _record.end())
            {
                itr = _record.emplace
                (
                    _self, [&](auto& nr)
                    {
                        nr.contract = _self;
                        nr.cur_group_count = 0;
                        nr.game_id = 0;
                    }
                );
                itr = _record.begin();
            }
            auto user_iterator = _players.find(username.value);
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

        //call this when user inactive
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
                    if (player.in_game)
                    {
                        player.lost_count = player.lost_count + 1;
                        player.prev_game = 0;
                    }
                    else if (player.game_id == itr->game_id && itr->cur_group_count != 0)
                        _record.modify
                        (
                            itr, _self, [&](auto& r)
                            {
                                r.cur_group_count =  r.cur_group_count - 1;
                            }
                        );
                    player.in_game = false;
                    player.iteration = -2;
                }
            );
        }

        //call this REPEATEDLY until in_game is true
        //table will be generated
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
                    player.in_game = true;
                    player.iteration = 0;
                    auto size = transaction_size();
                    char* buf = new char[size + strlen(player.seed.c_str())];
                    uint32_t read = read_transaction(buf, size);
                    strcpy(buf + read, player.seed.c_str());
                    capi_checksum256 hash;
                    sha256(buf, read + strlen(player.seed.c_str()), &hash);
                    delete [] buf;
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
                    for (i = 0; i < GAME_SIZE * 2; i++)
                        player.balls[i] = -1;
                }
            );
        }

        //call this periodically T > 4 seconds
        //For total of GAME_SIZE + 1 times
        //the last time is to end game
        [[eosio::action]]
        void genball(name username)
        {
            require_auth(username);
            auto ritr = _record.begin();
            eosio_assert(ritr != _record.end(), "Player does not exist");
            auto iterator = _players.find(username.value);
            eosio_assert(iterator != _players.end(), "Player does not exist");
            eosio_assert(iterator->in_game, "Player not in game!");
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
                    while (itr != ptable.end())
                    {
                        if (itr->game_id != player.game_id)
                            break;
                        if (itr->iteration == -1)
                        {
                            end_flag = false;
                            break;
                        }
                        itr++;
                    }
                    if (end_flag)
                    {
                        if (player.iteration == 0) //generate seed and first 5 balls
                        {
                            auto ptable = _players.get_index<"bygid"_n>();
                            auto itr = ptable.lower_bound(player.game_id);
                            std::string all_seed = "";
                            bool s_flag = true;
                            while (itr != ptable.end())
                            {
                                if (itr->game_id != player.game_id)
                                    break;
                                if (itr->iteration > player.iteration)
                                {
                                    s_flag = false;
                                    int i;
                                    for (i = 0; itr->balls[i] >= 0 && i < GAME_SIZE * 2; i++)
                                        player.balls[i] = itr->balls[i];
                                    player.iteration = itr->iteration;
                                    player.block_seed = itr->block_seed;
                                    break;
                                }
                                all_seed = all_seed + itr->seed;
                                itr++;
                            }
                            if (s_flag) //first 5 generation
                            {
                                player.iteration = player.iteration + 1;
                                capi_checksum256 h;
                                sha256(all_seed.c_str(), strlen(all_seed.c_str()), &h);
                                int8_t* t = new int8_t[GAME_SIZE * GAME_SIZE];
                                int i;
                                for (i = 0; i < GAME_SIZE * GAME_SIZE; i++)
                                    t[i] = i;
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
                                delete [] t;

                                auto size = transaction_size();
                                char* buf = new char[size + strlen(all_seed.c_str())];
                                uint32_t read = read_transaction(buf, size);
                                strcpy(buf + read, player.seed.c_str());
                                capi_checksum256 hash;
                                sha256(buf, read + strlen(player.seed.c_str()), &hash);
                                player.block_seed = hash;
                            }
                        }
                        else
                        {
                            player.iteration = player.iteration + 1;
                            int8_t* t = new int8_t[GAME_SIZE * GAME_SIZE];
                            int8_t i, k = 0;
                            for (i = 0; i < GAME_SIZE * GAME_SIZE; i++)
                                t[i] = i;
                            for (i = 0; i < GAME_SIZE; i++)
                                if (player.balls[GAME_SIZE + i] >= 0)
                                    t[player.balls[GAME_SIZE + i]] = -1;
                                else
                                    break;
                            i = player.iteration - 1;
                            uint32_t j = player.block_seed.hash[i * 4] << 24 | player.block_seed.hash[i * 4 + 1] << 16 | player.block_seed.hash[i * 4 + 2] << 8 | player.block_seed.hash[i * 4 + 3];
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
                            delete [] t;
                            eosio_assert(i + 1 == player.iteration, "We have sanity problem?");
                        }
                    }
                    else
                    {
                        player.iteration = -2;
                        player.lost_count = player.lost_count + 1;
                        player.prev_game = 0;
                        player.in_game = false;
                    }
                }
            );
        }
        
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
                            d &= player.board[i * GAME_SIZE + i];
                            e &= player.board[(i + 1) * GAME_SIZE - i - 1];
                            for (j = 0; j < GAME_SIZE; j++)
                            {
                                r &= player.board[i * GAME_SIZE + j];
                                c &= player.board[j * GAME_SIZE + i];
                            }
                            w |= r | c; // row & column
                        }
                        w |= d | e; //diagnal
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
                    else
                    {
                        player.iteration = -2;
                        player.lost_count = player.lost_count + 1;
                        player.prev_game = 0;
                        player.in_game = false;
                    }
                }
            );

        }

    [[eosio::action]]
    void hi( name user )
    {
        require_auth( user );
        print( "Hello, ", name{user} );
    }

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

EOSIO_DISPATCH( bingo, (login)(logout)(entergame)(genball)(declearwin)(hi)(erase))