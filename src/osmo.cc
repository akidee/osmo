#include <iostream>
/*#include <cmath>*/
#include "osmo.h"




int main (int c, char *argv[]) {

	if (c >= 3) {

		osmo::mongodb::osm2db(argv[1], argv[2]);
	}

	/*
	GeoObject* g = new GeoObject;
	g->p = GeoObject::P_node;
	g->id = 1000;
	Tag* t = new Tag;
	
	memccpy(t->k, "amenity", 0, 255);
	memccpy(t->v, "cafe", 0, 255);
	g->add_tag(t);
	g->add_tag(t);

	g->parts->add(g);

	std::cout << g->parts->refs[0]->id;
	*/

	std::cout << "ready";

	return 0;
}
