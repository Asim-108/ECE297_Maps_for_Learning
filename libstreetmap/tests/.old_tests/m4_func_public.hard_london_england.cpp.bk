#include <random>
#include <iostream>
#include <UnitTest++/UnitTest++.h>

#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m3.h"
#include "m4.h"

#include "unit_test_util.h"
#include "courier_verify.h"

using ece297test::relative_error;
using ece297test::courier_path_is_legal;


SUITE(hard_london_england_public) {
    TEST(hard_london_england) {
        std::vector<DeliveryInf> deliveries;
        std::vector<IntersectionIdx> depots;
        float turn_penalty;
        std::vector<CourierSubPath> result_path;
        bool is_legal;

        deliveries = {DeliveryInf(191002, 75030), DeliveryInf(338668, 226824), DeliveryInf(348712, 291283), DeliveryInf(68407, 269616), DeliveryInf(79121, 126306), DeliveryInf(29404, 118724), DeliveryInf(200996, 168942), DeliveryInf(202337, 65688), DeliveryInf(156986, 50082), DeliveryInf(293419, 124402), DeliveryInf(25790, 202542), DeliveryInf(98238, 156476), DeliveryInf(2382, 8047), DeliveryInf(245341, 84679), DeliveryInf(288683, 330568), DeliveryInf(41575, 10885), DeliveryInf(165589, 315264), DeliveryInf(173506, 266796), DeliveryInf(178811, 299628), DeliveryInf(338848, 266406), DeliveryInf(202483, 43881), DeliveryInf(16847, 170213), DeliveryInf(16703, 24735), DeliveryInf(351426, 127948), DeliveryInf(334062, 279215), DeliveryInf(304399, 264116), DeliveryInf(237480, 348610), DeliveryInf(344171, 164506), DeliveryInf(157702, 12777), DeliveryInf(290075, 265755), DeliveryInf(191764, 247673), DeliveryInf(93335, 336129), DeliveryInf(337987, 72827), DeliveryInf(161336, 262161), DeliveryInf(134227, 220293), DeliveryInf(311816, 287904), DeliveryInf(171006, 350351), DeliveryInf(222271, 77544), DeliveryInf(149750, 209657), DeliveryInf(188163, 130519), DeliveryInf(343563, 57592), DeliveryInf(315485, 96885), DeliveryInf(162693, 345824), DeliveryInf(45878, 57334), DeliveryInf(134959, 237905), DeliveryInf(320924, 115346), DeliveryInf(337882, 113917), DeliveryInf(110783, 39186), DeliveryInf(196736, 183441), DeliveryInf(21106, 336025), DeliveryInf(295786, 305818), DeliveryInf(319363, 292071), DeliveryInf(147874, 124124), DeliveryInf(317643, 148891), DeliveryInf(334229, 334225), DeliveryInf(252739, 73574), DeliveryInf(126203, 117471), DeliveryInf(164891, 307353), DeliveryInf(175225, 339972), DeliveryInf(102121, 172590), DeliveryInf(239938, 39694), DeliveryInf(254078, 199998), DeliveryInf(284475, 340560), DeliveryInf(56812, 163071), DeliveryInf(282462, 302000)};
        depots = {39};
        turn_penalty = 30.000000000;
        {
        	ECE297_TIME_CONSTRAINT(50000);
        	
        	result_path = travelingCourier(deliveries, depots, turn_penalty);
        }

        is_legal = courier_path_is_legal(deliveries, depots, result_path);
        CHECK(is_legal);

        if(is_legal) {
        	double path_cost = ece297test::compute_courier_path_travel_time(result_path, turn_penalty);
        	std::cout << "QoR hard_london_england: " << path_cost << std::endl;
        } else {
        	std::cout << "QoR hard_london_england: INVALID" << std::endl;
        }

    } //hard_london_england

} //hard_london_england_public

