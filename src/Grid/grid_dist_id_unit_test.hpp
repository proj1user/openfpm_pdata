#ifndef GRID_DIST_UNIT_TEST_HPP
#define GRID_DIST_UNIT_TEST_HPP

#include "grid_dist_id.hpp"
#include "data_type/scalar.hpp"

BOOST_AUTO_TEST_SUITE( grid_dist_id_test )

void print_test(std::string test, size_t sz)
{
	if (global_v_cluster->getProcessUnitID() == 0)
		std::cout << test << " " << sz << "\n";
}

BOOST_AUTO_TEST_CASE( grid_dist_id_domain_grid_unit_converter_test)
{
	// Domain
	Box<2,float> domain({0.0,0.0},{1.0,1.0});

	// Initialize the global VCluster
	init_global_v_cluster(&boost::unit_test::framework::master_test_suite().argc,&boost::unit_test::framework::master_test_suite().argv);

	Vcluster & v_cl = *global_v_cluster;

	// Skip this test on big scale
	if (v_cl.getProcessingUnits() >= 32)
		return;

	// Test several grid dimensions

	for (size_t k = 1024 ; k >= 2 ; k--)
	{
		BOOST_TEST_CHECKPOINT( "Testing grid k=" << k );

		// grid size
		size_t sz[2];
		sz[0] = k;
		sz[1] = k;

		// Ghost
		Ghost<2,float> g(0.01);

		// Distributed grid with id decomposition
		grid_dist_id<2, float, scalar<float>, CartDecomposition<2,float>> g_dist(sz,domain,g);

		// get the decomposition
		auto & dec = g_dist.getDecomposition();

		// check the consistency of the decomposition
		bool val = dec.check_consistency();
		BOOST_REQUIRE_EQUAL(val,true);

		// for each local volume
		// Get the number of local grid needed
		size_t n_grid = dec.getNLocalHyperCube();

		size_t vol = 0;

		// Allocate the grids
		for (size_t i = 0 ; i < n_grid ; i++)
		{
			// Get the local hyper-cube
			SpaceBox<2,float> sub = dec.getLocalHyperCube(i);

			Box<2,size_t> g_box = g_dist.getCellDecomposer().convertDomainSpaceIntoGridUnits(sub);

			vol += g_box.getVolumeKey();
		}

		v_cl.sum(vol);
		v_cl.execute();

		BOOST_REQUIRE_EQUAL(vol,sz[0]*sz[1]);
	}
}

void Test2D(const Box<2,float> & domain, long int k)
{
	long int big_step = k / 30;
	big_step = (big_step == 0)?1:big_step;
	long int small_step = 1;

	print_test( "Testing 2D grid k<=",k);

	// 2D test
	for ( ; k >= 2 ; k-= (k > 2*big_step)?big_step:small_step )
	{
		BOOST_TEST_CHECKPOINT( "Testing 2D grid k=" << k );

		//! [Create and access a distributed grid]

		// grid size
		size_t sz[2];
		sz[0] = k;
		sz[1] = k;

		float factor = pow(global_v_cluster->getProcessingUnits()/2.0f,1.0f/2.0f);

		// Ghost
		Ghost<2,float> g(0.01 / factor);

		// Distributed grid with id decomposition
		grid_dist_id<2, float, scalar<float>, CartDecomposition<2,float>> g_dist(sz,domain,g);

		// check the consistency of the decomposition
		bool val = g_dist.getDecomposition().check_consistency();
		BOOST_REQUIRE_EQUAL(val,true);

		// Grid sm
		grid_sm<2,void> info(sz);

		// get the domain iterator
		size_t count = 0;

		auto dom = g_dist.getDomainIterator();

		while (dom.isNext())
		{
			auto key = dom.get();
			auto key_g = g_dist.getGKey(key);

			g_dist.template get<0>(key) = info.LinId(key_g);

			// Count the point
			count++;

			++dom;
		}

		//! [Create and access a distributed grid]

		// Get the virtual cluster machine
		Vcluster & vcl = g_dist.getVC();

		// reduce
		vcl.sum(count);
		vcl.execute();

		// Check
		BOOST_REQUIRE_EQUAL(count,k*k);

		auto dom2 = g_dist.getDomainIterator();

		bool match = true;

		// check that the grid store the correct information
		while (dom2.isNext())
		{
			auto key = dom2.get();
			auto key_g = g_dist.getGKey(key);

			match &= (g_dist.template get<0>(key) == info.LinId(key_g))?true:false;

			++dom2;
		}

		BOOST_REQUIRE_EQUAL(match,true);

		g_dist.template ghost_get<0>();

		// check that the communication is correctly completed

		auto domg = g_dist.getDomainGhostIterator();

		// check that the grid with the ghost past store the correct information
		while (domg.isNext())
		{
			auto key = domg.get();
			auto key_g = g_dist.getGKey(key);

			// In this case the boundary condition are non periodic
			if (g_dist.isInside(key_g))
			{
				match &= (g_dist.template get<0>(key),info.LinId(key_g));
			}

			++domg;
		}
	}
}

void Test3D(const Box<3,float> & domain, long int k)
{
	long int big_step = k / 30;
	big_step = (big_step == 0)?1:big_step;
	long int small_step = 1;

	print_test( "Testing 3D grid k<=",k);

	// 2D test
	for ( ; k >= 2 ; k-= (k > 2*big_step)?big_step:small_step )
	{
		BOOST_TEST_CHECKPOINT( "Testing 3D grid k=" << k );

		// grid size
		size_t sz[3];
		sz[0] = k;
		sz[1] = k;
		sz[2] = k;

		// factor
		float factor = pow(global_v_cluster->getProcessingUnits()/2.0f,1.0f/3.0f);

		// Ghost
		Ghost<3,float> g(0.01 / factor);

		// Distributed grid with id decomposition
		grid_dist_id<3, float, scalar<float>, CartDecomposition<3,float>> g_dist(sz,domain,g);

		// check the consistency of the decomposition
		bool val = g_dist.getDecomposition().check_consistency();
		BOOST_REQUIRE_EQUAL(val,true);

		// Grid sm
		grid_sm<3,void> info(sz);

		// get the domain iterator
		size_t count = 0;

		auto dom = g_dist.getDomainIterator();

		while (dom.isNext())
		{
			auto key = dom.get();
			auto key_g = g_dist.getGKey(key);

			g_dist.template get<0>(key) = info.LinId(key_g);

			// Count the point
			count++;

			++dom;
		}

		// Get the virtual cluster machine
		Vcluster & vcl = g_dist.getVC();

		// reduce
		vcl.sum(count);
		vcl.execute();

		// Check
		BOOST_REQUIRE_EQUAL(count,k*k*k);

		bool match = true;

		auto dom2 = g_dist.getDomainIterator();

		// check that the grid store the correct information
		while (dom2.isNext())
		{
			auto key = dom2.get();
			auto key_g = g_dist.getGKey(key);

			match &= (g_dist.template get<0>(key) == info.LinId(key_g))?true:false;

			++dom2;
		}

		BOOST_REQUIRE_EQUAL(match,true);

		//! [Synchronize the ghost and check the information]

		g_dist.template ghost_get<0>();

		// check that the communication is correctly completed

		auto domg = g_dist.getDomainGhostIterator();

		// check that the grid with the ghost past store the correct information
		while (domg.isNext())
		{
			auto key = domg.get();
			auto key_g = g_dist.getGKey(key);

			// In this case the boundary condition are non periodic
			if (g_dist.isInside(key_g))
			{
				match &= (g_dist.template get<0>(key) == info.LinId(key_g))?true:false;
			}

			++domg;
		}

		BOOST_REQUIRE_EQUAL(match,true);

		//! [Synchronize the ghost and check the information]
	}
}

void Test2D_complex(const Box<2,float> & domain, long int k)
{
	typedef Point_test<float> p;

	long int big_step = k / 30;
	big_step = (big_step == 0)?1:big_step;
	long int small_step = 1;

	print_test( "Testing 2D complex grid k<=",k);

	// 2D test
	for ( ; k >= 2 ; k-= (k > 2*big_step)?big_step:small_step )
	{
		BOOST_TEST_CHECKPOINT( "Testing 2D complex grid k=" << k );

		//! [Create and access a distributed grid complex]

		// grid size
		size_t sz[2];
		sz[0] = k;
		sz[1] = k;

		float factor = pow(global_v_cluster->getProcessingUnits()/2.0f,1.0f/2.0f);

		// Ghost
		Ghost<2,float> g(0.01 / factor);

		// Distributed grid with id decomposition
		grid_dist_id<2, float, Point_test<float>, CartDecomposition<2,float>> g_dist(sz,domain,g);

		// check the consistency of the decomposition
		bool val = g_dist.getDecomposition().check_consistency();
		BOOST_REQUIRE_EQUAL(val,true);

		// Grid sm
		grid_sm<2,void> info(sz);

		// get the domain iterator
		size_t count = 0;

		auto dom = g_dist.getDomainIterator();

		while (dom.isNext())
		{
			auto key = dom.get();
			auto key_g = g_dist.getGKey(key);

			size_t k = info.LinId(key_g);

			g_dist.template get<p::x>(key) = 1 + k;
			g_dist.template get<p::y>(key) = 567 + k;
			g_dist.template get<p::z>(key) = 341 + k;
			g_dist.template get<p::s>(key) = 5670 + k;
			g_dist.template get<p::v>(key)[0] = 921 + k;
			g_dist.template get<p::v>(key)[1] = 5675 + k;
			g_dist.template get<p::v>(key)[2] = 117 + k;
			g_dist.template get<p::t>(key)[0][0] = 1921 + k;
			g_dist.template get<p::t>(key)[0][1] = 25675 + k;
			g_dist.template get<p::t>(key)[0][2] = 3117 + k;
			g_dist.template get<p::t>(key)[1][0] = 4921 + k;
			g_dist.template get<p::t>(key)[1][1] = 55675 + k;
			g_dist.template get<p::t>(key)[1][2] = 6117 + k;
			g_dist.template get<p::t>(key)[2][0] = 7921 + k;
			g_dist.template get<p::t>(key)[2][1] = 85675 + k;
			g_dist.template get<p::t>(key)[2][2] = 9117 + k;

			// Count the point
			count++;

			++dom;
		}

		//! [Create and access a distributed grid complex]

		// Get the virtual cluster machine
		Vcluster & vcl = g_dist.getVC();

		// reduce
		vcl.sum(count);
		vcl.execute();

		// Check
		BOOST_REQUIRE_EQUAL(count,k*k);

		auto dom2 = g_dist.getDomainIterator();

		bool match = true;

		// check that the grid store the correct information
		while (dom2.isNext())
		{
			auto key = dom2.get();
			auto key_g = g_dist.getGKey(key);

			size_t k = info.LinId(key_g);

			match &= (g_dist.template get<p::x>(key) == 1 + k)?true:false;
			match &= (g_dist.template get<p::y>(key) == 567 + k)?true:false;
			match &= (g_dist.template get<p::z>(key) == 341 + k)?true:false;
			match &= (g_dist.template get<p::s>(key) == 5670 + k)?true:false;
			match &= (g_dist.template get<p::v>(key)[0] == 921 + k)?true:false;
			match &= (g_dist.template get<p::v>(key)[1] == 5675 + k)?true:false;
			match &= (g_dist.template get<p::v>(key)[2] == 117 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[0][0] == 1921 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[0][1] == 25675 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[0][2] == 3117 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[1][0] == 4921 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[1][1] == 55675 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[1][2] == 6117 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[2][0] == 7921 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[2][1] == 85675 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[2][2] == 9117 + k)?true:false;

			++dom2;
		}

		BOOST_REQUIRE_EQUAL(match,true);

		//! [Synchronized distributed grid complex]

		g_dist.template ghost_get<p::x,p::y,p::z,p::s,p::v,p::t>();

		// check that the communication is correctly completed

		auto domg = g_dist.getDomainGhostIterator();

		// check that the grid with the ghost past store the correct information
		while (domg.isNext())
		{
			auto key = domg.get();
			auto key_g = g_dist.getGKey(key);

			// In this case the boundary condition are non periodic
			if (g_dist.isInside(key_g))
			{
				size_t k = info.LinId(key_g);

				match &= (g_dist.template get<p::x>(key) == 1 + k)?true:false;
				match &= (g_dist.template get<p::y>(key) == 567 + k)?true:false;
				match &= (g_dist.template get<p::z>(key) == 341 + k)?true:false;
				match &= (g_dist.template get<p::s>(key) == 5670 + k)?true:false;

				match &= (g_dist.template get<p::v>(key)[0] == 921 + k)?true:false;
				match &= (g_dist.template get<p::v>(key)[1] == 5675 + k)?true:false;
				match &= (g_dist.template get<p::v>(key)[2] == 117 + k)?true:false;

				match &= (g_dist.template get<p::t>(key)[0][0] == 1921 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[0][1] == 25675 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[0][2] == 3117 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[1][0] == 4921 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[1][1] == 55675 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[1][2] == 6117 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[2][0] == 7921 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[2][1] == 85675 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[2][2] == 9117 + k)?true:false;
			}

			++domg;
		}

		//! [Synchronized distributed grid complex]
	}
}

void Test3D_complex(const Box<3,float> & domain, long int k)
{
	typedef Point_test<float> p;

	long int big_step = k / 30;
	big_step = (big_step == 0)?1:big_step;
	long int small_step = 1;

	print_test( "Testing 3D grid complex k<=",k);

	// 2D test
	for ( ; k >= 2 ; k-= (k > 2*big_step)?big_step:small_step )
	{
		BOOST_TEST_CHECKPOINT( "Testing 3D complex grid k=" << k );

		// grid size
		size_t sz[3];
		sz[0] = k;
		sz[1] = k;
		sz[2] = k;

		// factor
		float factor = pow(global_v_cluster->getProcessingUnits()/2.0f,1.0f/3.0f);

		// Ghost
		Ghost<3,float> g(0.01 / factor);

		// Distributed grid with id decomposition
		grid_dist_id<3, float, Point_test<float>, CartDecomposition<3,float>> g_dist(sz,domain,g);

		// check the consistency of the decomposition
		bool val = g_dist.getDecomposition().check_consistency();
		BOOST_REQUIRE_EQUAL(val,true);

		// Grid sm
		grid_sm<3,void> info(sz);

		// get the domain iterator
		size_t count = 0;

		auto dom = g_dist.getDomainIterator();

		while (dom.isNext())
		{
			auto key = dom.get();
			auto key_g = g_dist.getGKey(key);

			size_t k = info.LinId(key_g);

			g_dist.template get<p::x>(key) = 1 + k;
			g_dist.template get<p::y>(key) = 567 + k;
			g_dist.template get<p::z>(key) = 341 + k;
			g_dist.template get<p::s>(key) = 5670 + k;
			g_dist.template get<p::v>(key)[0] = 921 + k;
			g_dist.template get<p::v>(key)[1] = 5675 + k;
			g_dist.template get<p::v>(key)[2] = 117 + k;
			g_dist.template get<p::t>(key)[0][0] = 1921 + k;
			g_dist.template get<p::t>(key)[0][1] = 25675 + k;
			g_dist.template get<p::t>(key)[0][2] = 3117 + k;
			g_dist.template get<p::t>(key)[1][0] = 4921 + k;
			g_dist.template get<p::t>(key)[1][1] = 55675 + k;
			g_dist.template get<p::t>(key)[1][2] = 6117 + k;
			g_dist.template get<p::t>(key)[2][0] = 7921 + k;
			g_dist.template get<p::t>(key)[2][1] = 85675 + k;
			g_dist.template get<p::t>(key)[2][2] = 9117 + k;

			// Count the point
			count++;

			++dom;
		}

		// Get the virtual cluster machine
		Vcluster & vcl = g_dist.getVC();

		// reduce
		vcl.sum(count);
		vcl.execute();

		// Check
		BOOST_REQUIRE_EQUAL(count,k*k*k);

		bool match = true;

		auto dom2 = g_dist.getDomainIterator();

		// check that the grid store the correct information
		while (dom2.isNext())
		{
			auto key = dom2.get();
			auto key_g = g_dist.getGKey(key);

			size_t k = info.LinId(key_g);

			match &= (g_dist.template get<p::x>(key) == 1 + k)?true:false;
			match &= (g_dist.template get<p::y>(key) == 567 + k)?true:false;
			match &= (g_dist.template get<p::z>(key) == 341 + k)?true:false;
			match &= (g_dist.template get<p::s>(key) == 5670 + k)?true:false;
			match &= (g_dist.template get<p::v>(key)[0] == 921 + k)?true:false;
			match &= (g_dist.template get<p::v>(key)[1] == 5675 + k)?true:false;
			match &= (g_dist.template get<p::v>(key)[2] == 117 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[0][0] == 1921 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[0][1] == 25675 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[0][2] == 3117 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[1][0] == 4921 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[1][1] == 55675 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[1][2] == 6117 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[2][0] == 7921 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[2][1] == 85675 + k)?true:false;
			match &= (g_dist.template get<p::t>(key)[2][2] == 9117 + k)?true:false;

			++dom2;
		}

		BOOST_REQUIRE_EQUAL(match,true);

		g_dist.template ghost_get<p::x,p::y,p::z,p::s,p::v,p::t>();

		// check that the communication is correctly completed

		auto domg = g_dist.getDomainGhostIterator();

		// check that the grid with the ghost past store the correct information
		while (domg.isNext())
		{
			auto key = domg.get();
			auto key_g = g_dist.getGKey(key);

			size_t k = info.LinId(key_g);

			// In this case the boundary condition are non periodic
			if (g_dist.isInside(key_g))
			{
				match &= (g_dist.template get<p::x>(key),1 + k)?true:false;
				match &= (g_dist.template get<p::y>(key),567 + k)?true:false;
				match &= (g_dist.template get<p::z>(key), 341 + k)?true:false;
				match &= (g_dist.template get<p::s>(key), 5670 + k)?true:false;

				match &= (g_dist.template get<p::v>(key)[0], 921 + k)?true:false;
				match &= (g_dist.template get<p::v>(key)[1], 5675 + k)?true:false;
				match &= (g_dist.template get<p::v>(key)[2], 117 + k)?true:false;

				match &= (g_dist.template get<p::t>(key)[0][0], 1921 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[0][1], 25675 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[0][2], 3117 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[1][0], 4921 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[1][1], 55675 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[1][2], 6117 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[2][0], 7921 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[2][1], 85675 + k)?true:false;
				match &= (g_dist.template get<p::t>(key)[2][2], 9117 + k)?true:false;
			}

			++domg;
		}

		BOOST_REQUIRE_EQUAL(match,true);
	}
}

BOOST_AUTO_TEST_CASE( grid_dist_id_iterator_test_use)
{
	// Domain
	Box<2,float> domain({0.0,0.0},{1.0,1.0});

	// Initialize the global VCluster
	init_global_v_cluster(&boost::unit_test::framework::master_test_suite().argc,&boost::unit_test::framework::master_test_suite().argv);

	long int k = 1024*1024*global_v_cluster->getProcessingUnits();
	k = std::pow(k, 1/2.);

	Test2D(domain,k);
	Test2D_complex(domain,k);
	// Domain
	Box<3,float> domain3({0.0,0.0,0.0},{1.0,1.0,1.0});

	k = 128*128*128*global_v_cluster->getProcessingUnits();
	k = std::pow(k, 1/3.);
	Test3D(domain3,k);
	Test3D_complex(domain3,k);
}

BOOST_AUTO_TEST_SUITE_END()

#endif
