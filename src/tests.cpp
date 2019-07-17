#include <iostream>
#include <assert.h>
#include <optional>
#include <vector>
#include <cstdint>
#include <string>
#include <tuple>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <alloca.h>

#include "api.h"

std::string stringff(const char *format, ...)
{
    va_list arg_list;
    va_start(arg_list, format);

    // SUSv2 version doesn't work for buf NULL/size 0, so try printing
    // into a small buffer that avoids the double-rendering and alloca path too...
    char short_buf[256];
    const size_t needed = vsnprintf(short_buf, sizeof short_buf,
                                    format, arg_list) +
                          1;
    if (needed <= sizeof short_buf)
        return short_buf;

    // need more space...

    // OPTION 1
    std::string result(needed, ' ');
    vsnprintf(result.data(), needed, format, arg_list);
    return result; // RVO ensures this is cheap
}

std::string string_hex(std::vector<std::uint8_t> const &input)
{
    std::string s;
    for (auto it = input.cbegin(); it != input.cend(); it++)
    {
        auto const byte = *it;
        s += stringff("%x", byte >> 4);
        s += stringff("%x", byte & 0xf);
    }
    return s;
}

void api_test(std::vector<std::uint8_t> const &input, std::optional<std::vector<std::uint8_t>> const &output, std::string const &name)
{
    auto const result = run(input);
    if (auto answer = std::get_if<0>(&result))
    {
        if (output)
        {
            if (output.value() == *answer)
            {
                std::cout << "Ok: " << name << std::endl;
            }
            else
            {

                std::cout << "Err: Returned value differs: " << name << std::endl;
                std::cout << "Expected: " << string_hex(output.value()) << std::endl;
                std::cout << "Got     : " << string_hex(*answer) << std::endl;
            }
        }
        else
        {
            std::cout << "Err: Run returned value: " << name << std::endl;
        }
    }
    else
    {
        if (output)
        {
            std::cout << "Err: Run errored (" << std::get<1>(result) << "): " << name << std::endl;
        }
        else
        {
            std::cout << "Ok: " << name << std::endl;
        }
    }
}

std::uint8_t parse_hex_char(char c)
{
    std::uint8_t byte = 0;
    switch (c)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        byte = c - '0';
        break;
    }
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    {
        byte = 10 + c - 'a';
        break;
    }
    default:
        std::cout << "Not a hex char: " << c << std::endl;
        break;
    }
    return byte;
}

// Parses data in hexadecimal caracters
std::vector<std::uint8_t> parse_hex(std::string const &input)
{
    std::vector<std::uint8_t> data;
    for (auto it = input.cbegin(); it != input.cend();)
    {
        auto const high = parse_hex_char(*it);
        it++;
        auto const low = parse_hex_char(*it);
        it++;

        data.push_back((high << 4) | low);
    }

    return data;
}

void tests()
{
    {
        auto const input = parse_hex("0268259362ffe4eeeab4198826f2a6123c684cb9e1cb9776a641521a8e584ad990b3a95a828a22b542cf9c2e9e5fb1f800bc266b1bc34b4485c4cad7d5dcb588ead513c35d1110f39ed3e6f6701f1344c197b9e2d148ce4ffc5d00c209f75a4e68ff10a2c9d2cf7c567f000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000024603acd52f0a8a647b7d70f565298a1b3e4a5349641b7c7c35db0fb47fbe2e757138c5c08dd21d76b9954307b68e400bb3f912c3b60c0b7856ba09d6e396a39852de12492760f11bbd8b79002c5eea2c3757d43fb6bd5dbe7eb38c857a8fadad489188185712e77792c2fca45adb1a75c95f19c7556ede577fb6483c59af5b01f83d3abe6795f3ab19980e1aca89f70025ae6eaa1e276afcf5a852cf3106be6a0614c27f812f83c173d45c04de620711b07220f30996f3652eb0e61b07df94b8e9b0717f5858564efe837d6164f6ca77e0babbbe1f44c780e0caedea5839cc114034f7f20c5f937edca1b026d7c29542222de53f9c794dc976b983bbfdbe18868a2ea1a77705688748469e13331203dcaf10ce79816d7b017c26dbec741af0a354ebfba7f9a82ab4c41ddcf0ebfd751fb0ab731aa6ffd42e1f38f35fd682bd1bdf6a47678818bd8a823cbf323c31040d5a2ceca884850229534825f324");
        auto const output = parse_hex("0fccd33a00d85ba0ee3ce8571406aa2e2a8da22d682b855c0bd647513723ecc1e8595e9146d382e9ea3b9e8287a9e5776fa3ac3ac7a97c0595cc1bda5d3ef1efe4a9152d9bf3998b435d8a8d698e1186f5c856a9c418b97404d6f33c080ef7925fde8f63f7e4ef241d2c5778c7ba25b4a368c53649da84f8d25d1cde12ecdaf28c9756571355f9ac88ea7c1ed3f1baf902c6bd7b3e9cbfde4d1f72d74dba5caf003a729f7d9de60b1bcfb8acd51ccd2b663b81d2273ef8301c952e065fc90aadb06a625b1947687fa6514a700882f4d1");
        api_test(input, output, "G1 multiplication: 0");
    }
    // {
    //     auto const input = parse_hex("05202912811758d871b77a9c3635c28570dc020576f9fc2719d8d0494439162b2b89022912811758d871b77a9c3635c28570dc020576f9fc2719d8d0494439162b2b8400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001603e65409b693c8a08aeb3478d10aa3732a6672ba06d11a5e2f8c64a9d8fa1297206227e6954ea12740ea14061e0aa59cf1d99242e568033ab6f8d712143d4b6b92a45a96b0b740226985ee1577c2fc2a8452cbda7e591adcf688b458231e35ab5ac92aee15e0ea82655a1dce39bff837274e66f423ca0d55c72dcf09d25998bc74507f5d01960cc76234c40c3a897a11a4a34faba6ca02ea0ea9c00da6d920d377821c7b2c9c0df294c93223");
    //     auto const output = parse_hex("264b5c0da7075851dd74b2a727d0ccf95f21d55f946453caf6b701dab9d1e7ce0d664cc12991cfc0765b66367b12aa3032087dd7329756d305fc5f6a0bc81645126356a11508b70a268ddfa6fd0736cad1722c646c43b430b915fb6d6df3b8c20197e879c06be063bab5fc52440a33c295ff6c8fc46c41726c2790fa0b294fbc");
    //     api_test(input, output, "G2 multiplication: 0");
    // }
    // {
    //     auto const input = parse_hex("0568259362ffe4eeeab4198826f2a6123c684cb9e1cb9776a641521a8e584ad990b3a95a828a22b542cf9c2e9e5fb1f800bc266b1bc34b4485c4cad7d5dcb588ead513c35d1110f39ed3e6f6701f1344c197b9e2d148ce4ffc5d00c209f75a4e68ff10a2c9d2cf7c567f02259362ffe4eeeab4198826f2a6123c684cb9e1cb9776a641521a8e584ad990b3a95a828a22b542cf9c2e9e5fb1f800bc266b1bc34b4485c4cad7d5dcb588ead513c35d1110f39ed3e6f6701f1344c197b9e2d148ce4ffc5d00c209f75a4e68ff10a2c9d2cf7c567e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004603acd52f0a8a647b7d70f565298a1b3e4a5349641b7c7c35db0fb47fbe2e757138c5c08dd21d76b9954307b68e400bb3f912c3b60c0b7856ba09d6e396a39852de12492760f1036f88a4149393b994f6d0c31c8a6d50198a1d60060aa4161b7b242f020e3d0d8a916ae1a565e7872c91e04408dcbdba757cffc81d41b0cbd4fe1406e01eecdd560e49d3fcd392fb75f280d1336d0be250578168df0190c448c3db4c65ba92a06fb81588a28f7b050f7e1c5d17bbc074408fa7880851b1cb6724bcaefaab8da3a5062a90c8c73bf946e309dab187b06b2a52042747efafea134b60889cab1fa6c368da83e313098c9ca2251bf55f1269b55dcffd2409a7c4e65f6149f20611ae0a5399cd2eee0f2ac93e9408d5b991dd1701124102f19bac5dde49eb976306060e1049c2eb5ab73d5e1bba8cca8c66b557773b308b45a843b5daf1a2b7a968fafe8ff96053e1bc3198c304b3f7d86ea7eae2592caec9d79ed97645c175226b8962d9d27e78943b0f29dfe7117231ddf2c05f6ed2a2cc49620aa5bf3a99e736dccf546a844ed447b066870d36a337430af91102f379a90b80c3cacbf850c809942d6a0410425e0c98766103a25d8ee911934e7d0ea50c94ec50f811e48e3a37934703bb714135dee961bd88aeca048659390c8a7dd497e31774a9978dc51b3607027aca1e946ef4dbd5f2a2df6a3a5fe9c1283d446900eef91a185ad09d63ba518ffb1f0ebf3ae3abbaf17dbb7c0d06b45bf700126c2a5a31ab268b33961354a6732f199a35b6");
    //     auto const output = parse_hex("04d464743c373711b8b34a74853a935594bdf40f2b8312d0e2aacad7ed0dfcdee58c8f06d4c3704946f7987fbe7f4fc4554ae790815fdbe5c97c6e1ae1886f5bfef331d973b1f258ceee5724a2ff0498ef4beee8f969b0cf7b65f1c9d6c86f07d42ea107e0758dce006ff974b07be4b048215d88ac1064e8d33e031eeb6e3b4afbe812b1d764d57025b92826275babf3700688e83680fbcf994e167a4e2840724f91028810680435a49041f5d10090eba0da199925932a2acb397395b2c9a77053979ae3c46da96660d6cd49482f76441ca5f0cce32be561fb2b354495fd0e5de3f460ca61435e828f0368213fb0bddec1b0552e5be8833e8b809521cc73f06476aede3d7853a82d8683d0f027d5765270e1fa3fb19cef4a0cff3d8aa37bd6107b8b82d2aa800d2fcbdea93585e15541b897b136791457280582ed337b826fe3cd42c391fa2dec9dd9a5684e34a342a9c0110f40e3545b8b20cd01b73865102880646a146f4380add11ed004c379369a7ffe36ebf5f48ec068fd99abd31ad8ef281b10249cda61dd694564e28ec2b51d91837f1d9e307fed0765a46fd87a0782");
    //     api_test(input, output, "G2 multiplication: 1");
    // }
    // {
    //     auto const input = parse_hex("0531026d331f7ce9cdc5e48d73aeedf0a1d7f7870b788046ec2aff712eda78d3fe42dfb8fe05c47ae860b05717583ae170045d02026d331f7ce9cdc5e48d73aeedf0a1d7f7870b788046ec2aff712eda78d3fe42dfb8fe05c47ae860b05717583ae170045b000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002103c192577dfb697d258e5f48f4c3f36bb518d0ea9b498ca3559dfb03a2c685a5290101ffde193f98185f975ffb1372580fd7a93194e7a50a5cf7852f292455e0a34451c82bd7901a3de0c2a3479e716cbbdc00ac30a79956814c1a0e970fb255184f367d122abb6f3f9627ac2c2c97a68379d16883dfaf2549e3a3dce33f3bea1247df01460c7673ee5b7a2db0e9452d1b7fbb502e3f999f1b1dcd31e7062346408863c7f973b94e34c8b55c32bf5e829c664688005aa657b27ade7a8f4b1cabea24ce403f3ce23f7274e2569c90bcb97a4c04434c78438560cec76719bde389aedc17011301d82faa3935ba82cc72fca999eaa64a3e13f4b70e08eae12074e1841ea7e78c19");
    //     auto const output = parse_hex("004b6e56e5c05c0326653db28ee57c4155a8257dfe50695f14e5bb6c15113429f154aa5cd0a75a9b7db8b7d9c878a11515024e5f4800afc640288d68e5c9c2209795f8789465e0b0f49d1c047341cc61b52068eff7186db42de70eae178e6a08e75b0096d93f4253ae722fd5833f508110b4154a4ed440401174ddeaec89ac236a9b8bb325ac07253e64b18e8cce66ecac4c1f01c2721afe0cb38260be4cf450e317e4c60e191b3c98348aae52d7b906bb9eacbb99ea7064ccac70b63a6926794ff538ce");
    //     api_test(input, output, "G2 multiplication: 2");
    // }
    // {
    //     auto const input = parse_hex("052885fc1f752ac6f59f216571afe4d8db11c41bd3cc740f6270bb7cc3ac835a00e7d7ab6e9c5e72a30b0285fc1f752ac6f59f216571afe4d8db11c41bd3cc740f6270bb7cc3ac835a00e7d7ab6e9c5e72a30a000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001b36770e180fecfb20096cf26d0dff3151536877d570787a3b64bc414970e14721ec542f65f4e068855530505e32a40f57a2148c619e7e02f50e2e1debfff50870db8de684d6937638d8694890d9ceec508e14719211a22093040ac5e6300ffc8cb6ebf0c4eaafd21a6665a802bb174ef38349339ff9730404019a83c2d6d9776ec43f04e7c07285cf797111a6acbf22578e674d578e3b4204772e6f73a6e3ad6209cc7a020cd9351d85da0036c7af0fbf8154ba7be8aef2ee26632a2510336cd8552dba1855019ac0a6ab2ef0e6a5c5345884e3086826");
    //     auto const output = parse_hex("7ccf8ead202f00683db10087c511a2d6d43aa1fe8efa5de19e86368b13d77969e370abf3a4a8cd876410835eb0def110a0aabfc237af1fee6bbce8ea171e31ce154e1b0554471189638918c98eec3b645f52092aeea6aa97da5dc5e06a04c9daff33e3622a8de62d55c1bafa3ba165899fc1455fba91c34e3440f44dbacd555e2860e3cf07138bbf63ab5c9796de21913e36f2aac71e38d19e82978f43420dc4");
    //     api_test(input, output, "G2 multiplication: 3");
    // }
}

int main()
{
    // // Api
    // std::vector<std::uint8_t> input = {0, 1, 2, 3, 4};
    // auto result = run(input);
    // if (auto err = std::get_if<1>(&result))
    // {
    //     std::cout
    //         << "Err: " << *err << std::endl;
    // }
    // else
    // {
    //     std::cout
    //         << "Ok" << std::endl;
    // }
    std::cout << "Starting tests" << std::endl;
    tests();
    std::cout << "Done testing" << std::endl;
}