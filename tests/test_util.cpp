//
// Created by svend on 021, 21-02-2023.
//
#include <boost/test/unit_test.hpp>
#include "../util/util.h"
#include "../util/otUtil/otUtil.h"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Testing_generateRandomLabels )

    vector<tuple<vector<uint64_t>, vector<uint64_t>>> inputWiresLabels =
            vector<tuple<vector<uint64_t>, vector<uint64_t>>>(2);
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> outputWiresLabels =
            vector<tuple<vector<uint64_t>, vector<uint64_t>>>(2);
    vector<uint64_t> globalDelta = vector<uint64_t>(2);
    auto output = util::generateRandomLabels(128, globalDelta, inputWiresLabels);
    vector<uint64_t> output0 = get<0>(output);
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> output1 = get<1>(output);


    BOOST_AUTO_TEST_CASE( test_globalDelta )
    {
        BOOST_TEST( output0[0] != 0);
        BOOST_TEST( globalDelta[0] != 0);
        BOOST_TEST( (output0[0] & 1) == 1);

    }
    BOOST_AUTO_TEST_CASE( test_labelPointAndPermute )
    {
        tuple<vector<uint64_t>, vector<uint64_t>> wiresLabels =
                tuple<vector<uint64_t>, vector<uint64_t>>(vector<uint64_t>(2),
                                                          vector<uint64_t>(2));
        util::labelPointAndPermute(128, wiresLabels, output0);
        BOOST_TEST(get<0>(wiresLabels)[0] != 0);
        BOOST_TEST(get<0>(wiresLabels)[1] != 0);
        BOOST_TEST(get<1>(wiresLabels)[0] != 0);
        BOOST_TEST(get<1>(wiresLabels)[1] != 0);
        for (int i = 0; i < 10; ++i) {
            tuple<vector<uint64_t>, vector<uint64_t>> wiresLabels =
                    tuple<vector<uint64_t>, vector<uint64_t>>(vector<uint64_t>(2),
                                                              vector<uint64_t>(2));
            util::labelPointAndPermute(128, wiresLabels, output0, 0);
            //test color bit
            BOOST_TEST((get<0>(wiresLabels)[0] & 1) == 0);
            BOOST_TEST((get<1>(wiresLabels)[0] & 1) == 1);
            BOOST_TEST(util::bitVecXOR(get<0>(wiresLabels), get<1>(wiresLabels)) == output0);

        }
        for (int i = 0; i < 10; ++i) {
            tuple<vector<uint64_t>, vector<uint64_t>> wiresLabels =
                    tuple<vector<uint64_t>, vector<uint64_t>>(vector<uint64_t>(2),
                                                              vector<uint64_t>(2));
            util::labelPointAndPermute(128, wiresLabels, output0, 1);
            //test color bit
            BOOST_TEST((get<0>(wiresLabels)[0] & 1) == 1);
            BOOST_TEST((get<1>(wiresLabels)[0] & 1) == 0);
            BOOST_TEST(util::bitVecXOR(get<0>(wiresLabels), get<1>(wiresLabels)) == output0);

        }
    }


BOOST_AUTO_TEST_SUITE_END()