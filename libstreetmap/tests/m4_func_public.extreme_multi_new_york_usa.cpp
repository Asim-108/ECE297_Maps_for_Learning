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


SUITE(extreme_multi_new_york_usa_public) {
    TEST(extreme_multi_new_york_usa) {
        std::vector<DeliveryInf> deliveries;
        std::vector<IntersectionIdx> depots;
        float turn_penalty;
        std::vector<CourierSubPath> result_path;
        bool is_legal;

        deliveries = {DeliveryInf(128908, 2693), DeliveryInf(35252, 99348), DeliveryInf(22251, 159570), DeliveryInf(34721, 92886), DeliveryInf(49371, 121363), DeliveryInf(125062, 5650), DeliveryInf(74078, 218246), DeliveryInf(258859, 45445), DeliveryInf(53030, 115492), DeliveryInf(233281, 185611), DeliveryInf(22251, 99348), DeliveryInf(297157, 229087), DeliveryInf(290213, 99348), DeliveryInf(53030, 180229), DeliveryInf(258859, 30746), DeliveryInf(74078, 35752), DeliveryInf(141497, 176225), DeliveryInf(294298, 283549), DeliveryInf(53030, 277005), DeliveryInf(49371, 284786), DeliveryInf(219576, 122761), DeliveryInf(258859, 175757), DeliveryInf(8865, 99348), DeliveryInf(302011, 176349), DeliveryInf(38568, 252303), DeliveryInf(258859, 207040), DeliveryInf(74078, 122761), DeliveryInf(141763, 159570), DeliveryInf(39102, 7601), DeliveryInf(304742, 145782), DeliveryInf(258859, 40952), DeliveryInf(53030, 289154), DeliveryInf(125696, 137121), DeliveryInf(258859, 207040), DeliveryInf(275288, 213309), DeliveryInf(239617, 238002), DeliveryInf(65088, 104864), DeliveryInf(284800, 64548), DeliveryInf(12315, 59733), DeliveryInf(137316, 295755), DeliveryInf(202675, 175952), DeliveryInf(304742, 69899), DeliveryInf(22251, 196912), DeliveryInf(190494, 240546), DeliveryInf(200667, 288605), DeliveryInf(232491, 8072), DeliveryInf(143738, 43213), DeliveryInf(228489, 225934), DeliveryInf(159466, 66916), DeliveryInf(12315, 168859), DeliveryInf(169192, 35636), DeliveryInf(53030, 158606), DeliveryInf(252619, 2768), DeliveryInf(12315, 188168), DeliveryInf(41657, 176225), DeliveryInf(32306, 43213), DeliveryInf(74078, 158606), DeliveryInf(69851, 130142), DeliveryInf(140197, 239384), DeliveryInf(211553, 277005), DeliveryInf(258859, 157713), DeliveryInf(265582, 231746), DeliveryInf(74229, 135253), DeliveryInf(267850, 175757), DeliveryInf(265582, 106753), DeliveryInf(192905, 277005), DeliveryInf(70848, 176225), DeliveryInf(41657, 216638), DeliveryInf(53030, 5571), DeliveryInf(174371, 122761), DeliveryInf(158258, 55030), DeliveryInf(143591, 284786), DeliveryInf(265582, 297362), DeliveryInf(236590, 263087), DeliveryInf(119708, 122761), DeliveryInf(133752, 2033), DeliveryInf(49371, 95711), DeliveryInf(202675, 121363), DeliveryInf(115650, 6578), DeliveryInf(290446, 290145), DeliveryInf(30660, 244807), DeliveryInf(87895, 175757), DeliveryInf(53030, 252912), DeliveryInf(32306, 113876), DeliveryInf(53030, 158606), DeliveryInf(254014, 217019), DeliveryInf(68136, 175952), DeliveryInf(278557, 178504), DeliveryInf(129400, 111354), DeliveryInf(70204, 201678), DeliveryInf(218200, 159570), DeliveryInf(217498, 130142), DeliveryInf(114689, 94490), DeliveryInf(48439, 162123), DeliveryInf(96985, 284786), DeliveryInf(87497, 78064), DeliveryInf(162350, 302101), DeliveryInf(45908, 242636), DeliveryInf(150751, 24389), DeliveryInf(248656, 174976), DeliveryInf(258859, 263087), DeliveryInf(230858, 37498), DeliveryInf(92193, 303881), DeliveryInf(104420, 207040), DeliveryInf(84867, 231746), DeliveryInf(78595, 212297), DeliveryInf(216422, 223830), DeliveryInf(1820, 10929), DeliveryInf(141763, 173614), DeliveryInf(142510, 211615), DeliveryInf(57677, 111295), DeliveryInf(162666, 156625), DeliveryInf(29465, 289154), DeliveryInf(41657, 207040), DeliveryInf(45908, 58260), DeliveryInf(22251, 233287), DeliveryInf(122121, 43213), DeliveryInf(38758, 175952), DeliveryInf(141763, 67655), DeliveryInf(74078, 131474), DeliveryInf(882, 302101), DeliveryInf(169005, 224746), DeliveryInf(189287, 85854), DeliveryInf(12315, 159570), DeliveryInf(150751, 231746), DeliveryInf(12315, 186406), DeliveryInf(304742, 175757), DeliveryInf(202675, 137121), DeliveryInf(110921, 76758), DeliveryInf(258859, 131474), DeliveryInf(231286, 270805), DeliveryInf(141763, 204577), DeliveryInf(10629, 111582), DeliveryInf(53030, 144878), DeliveryInf(198007, 270634), DeliveryInf(265582, 43213), DeliveryInf(141763, 252783), DeliveryInf(150751, 155005), DeliveryInf(290589, 289154), DeliveryInf(19069, 226084), DeliveryInf(12315, 188591), DeliveryInf(49371, 297362), DeliveryInf(223099, 299191), DeliveryInf(265582, 284786), DeliveryInf(224962, 99348), DeliveryInf(273001, 121363), DeliveryInf(12315, 58044), DeliveryInf(216035, 131474), DeliveryInf(233524, 122710), DeliveryInf(11480, 159570), DeliveryInf(32306, 40398), DeliveryInf(184575, 290057), DeliveryInf(251382, 4217), DeliveryInf(41657, 160816), DeliveryInf(107545, 241770), DeliveryInf(150751, 88024), DeliveryInf(22251, 82518), DeliveryInf(271894, 110709), DeliveryInf(258859, 52423), DeliveryInf(259669, 12387), DeliveryInf(257893, 22708), DeliveryInf(95707, 122761), DeliveryInf(5598, 192459), DeliveryInf(41657, 246252), DeliveryInf(225760, 11956), DeliveryInf(30785, 121363), DeliveryInf(41657, 240383), DeliveryInf(22251, 248635), DeliveryInf(304742, 240546), DeliveryInf(179695, 274216), DeliveryInf(10773, 224096), DeliveryInf(265582, 268618), DeliveryInf(187696, 147697), DeliveryInf(74559, 15922), DeliveryInf(141763, 38052), DeliveryInf(184704, 43213), DeliveryInf(76435, 297362), DeliveryInf(265582, 284786), DeliveryInf(179827, 143100), DeliveryInf(261212, 108016), DeliveryInf(217086, 90999), DeliveryInf(83868, 144878), DeliveryInf(201589, 216638), DeliveryInf(298885, 73164), DeliveryInf(202675, 297362), DeliveryInf(28575, 74657), DeliveryInf(141763, 97834), DeliveryInf(254658, 113502), DeliveryInf(22251, 274970), DeliveryInf(32318, 130142), DeliveryInf(81044, 176225), DeliveryInf(289423, 280069)};
        depots = {10594, 258237};
        turn_penalty = 30.000000000;
        {
        	ECE297_TIME_CONSTRAINT(50000);
        	
        	result_path = travelingCourier(deliveries, depots, turn_penalty);
        }

        is_legal = courier_path_is_legal(deliveries, depots, result_path);
        CHECK(is_legal);

        if(is_legal) {
        	double path_cost = ece297test::compute_courier_path_travel_time(result_path, turn_penalty);
        	std::cout << "QoR extreme_multi_new_york_usa: " << path_cost << std::endl;
        } else {
        	std::cout << "QoR extreme_multi_new_york_usa: INVALID" << std::endl;
        }

    } //extreme_multi_new_york_usa

} //extreme_multi_new_york_usa_public

