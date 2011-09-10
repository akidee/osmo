#include <vector>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <math.h>

#include "client/dbclient.h"

using namespace mongo;




namespace osmo {
	

	class Tag {
	public:


		// TYPES
		static const int K_MAXLEN = 4 * 255 + 1;
		static const int V_MAXLEN = 4 * 255 + 1;


		// DATA
		char k[K_MAXLEN];
		char v[V_MAXLEN];


		// BASIC METHODS
		bool set_attr (const char *a, const char *v) {

			// Accept labels names from OSM schema as well
			if (!strcmp(a, "k")) {

				if (!memccpy(k, v, 0, K_MAXLEN))
            			throw std::length_error("k too long");

            		// MongoDB key restriction
            		if (k[0] == '$') k[0] = ' ';
            		char* f;
            		while ((f = strchr(k, '.')) != NULL) {

					f[0] = ':';
            		}
            		
            }
            else if (!strcmp(a, "v")) {

				if (!memccpy(this->v, v, 0, V_MAXLEN))
            			throw std::length_error("v too long");
            }
            else
            		return false;

            	return true;
		}

		void appendToBsonBuilder (BSONObjBuilder* b) {

			b
				->append(k, v);
		}

		BSONObj getBson () {

			BSONObjBuilder b;
			b
				.append("k", k)
				.append("v", v);
			return b.obj();
		}
	};
	

	class User {
	public:


		// TYPES
		typedef int ID;
		static const int NAME_MAXLEN = 4 * 255 + 1;


		// PARSERS
		#define OSMO_USER_ID(s) atoi(s)


		// DATA
		ID id;
		char name[NAME_MAXLEN];


		// BASIC METHODS
		User () {

			reset();
		}

		void reset () {

			id = 0;
			name[0] = '\0';
		}
		
		bool set_attr (const char *a, const char *v) {

			// Accept labels names from OSM schema as well
			if (!strcmp(a, "uid") || !strcmp(a, "id")) {

				id = OSMO_USER_ID(v);
            }
            else if (!strcmp(a, "user") || !strcmp(a, "name")) {

                if (!memccpy(name, v, 0, NAME_MAXLEN))
            			throw std::length_error("timestamp too long");
            }
            else
            		return false;

            	return true;
		}

		BSONObj getBson () {

			BSONObjBuilder b;
			b
				.append("id", id)
				.append("name", name);
			return b.obj();
		}
	};


	class GeoObjectRef { 
	public:


		// TYPES
		static const int P_MAXLEN = 1 + 1;
		typedef long long ID; // Correct: long long


		// PARSERS
		#define OSMO_GEOOBJECTREF_ID(s) atoll(s) // Correct: atoll


		// DATA
		char p[P_MAXLEN];
		ID id;


		// BASIC METHODS
		GeoObjectRef () {

			reset();
		}

		void reset () {

			p[0] = '\0';
			id = 0;
		}
		
		virtual bool set_attr (const char* a, const char* v) {

			if (!strcmp(a, "ref") || !strcmp(a, "id")) {

				id = OSMO_GEOOBJECTREF_ID(v);
            }
            else if (!strcmp(a, "p") || !strcmp(a, "type")) {

				p[0] = v[0];
				p[1] = '\0';
            }
            else
            		return false;

            return true;
		}

		void appendToBsonBuilder (BSONObjBuilder* b) {

			b
				->append("p", p)
				.append("id", (int)id); // Correct: (long long)
		}

		virtual BSONObj getBson () {

			BSONObjBuilder b;
			appendToBsonBuilder(&b);
			return b.obj();
		}
	};


	class GeoObjectRel {
	public:

		const static int ROLE_MAXLEN = 4 * 255 + 1;


		char role[ROLE_MAXLEN];


		GeoObjectRel () {};


		bool set_attr (const char *a, const char *v) {

			if (!strcmp(a, "role")) {

				if (!memccpy(role, v, 0, ROLE_MAXLEN))
            			throw std::length_error("role too long");
            }
            else
            		return false;

            return true;
		}

		BSONObj getBson () {

			BSONObjBuilder b;
			b
				.append("role", role);
			return b.obj();
		}
	};


	class GeoObjectView {
	public:


		// TYPES
		typedef std::vector<GeoObjectRef> LIST;
		typedef std::vector<GeoObjectRel> INFO;
	

		// DATA
		LIST list;
		INFO info;


		//
		void reset () {

			list.clear();
			info.clear();
		}


		//
		void add (GeoObjectRef ref, GeoObjectRel rel) {

			list.push_back(ref);
			info.push_back(rel);
		}

		void add (GeoObjectRef ref) {

			list.push_back(ref);
		}

		BSONObj getBson () {

			int len;

			BSONObjBuilder b;
			
				BSONArrayBuilder listB;
				len = list.size();
				for (int i = 0; i < len; i++) {

		            listB.append(list[i].getBson());
		        }
		        b.append("list", listB.arr());

				BSONArrayBuilder infoB;
					len = info.size();
					for (int i = 0; i < len; i++) {

			            infoB.append(info[i].getBson());
			        }
				b.append("info", infoB.arr());
			
			return b.obj();
		}
	};


	class GeoObject : public GeoObjectRef {
	public:


		// TYPES
		typedef User* USER;
		typedef int C; // Correct: long long
		static const int T_MAXLEN = 20 + 1;
		typedef int V;
		typedef std::vector<Tag> TAGS;
		typedef double LON;
		typedef double LAT;
		typedef GeoObjectView* PARTS;


		// PARSERS
		#define OSMO_GEOOBJECT_C(s) atoi(s) // Correct: atoll
		#define OSMO_GEOOBJECT_V(s) atoi(s)
		#define OSMO_GEOOBJECT_LON(s) atof(s)
		#define OSMO_GEOOBJECT_LAT(s) atof(s)


		// DATA
		USER user;
		C c;
		char t[T_MAXLEN];
		V v;
		TAGS tags;
		LON lon;
		LAT lat;
		PARTS parts;


		// BASIC METHODS
		GeoObject () : GeoObjectRef () {

			user = new User;
			parts = new GeoObjectView;
			reset();
		}

		~GeoObject () {

			delete user;
			delete parts;
		}

		void reset () {

			GeoObjectRef::reset();
			user->reset();
			c = 0;
			t[0] = '\0';
			v = 0;
			tags.clear();
			lon = NAN;
			lat = NAN;
			parts->reset();
		}

		virtual bool set_attr (const char* a, const char* v) {

			if (!strcmp(a, "changeset") || !strcmp(a, "changeset_id") || !strcmp(a, "c")) {

            		c = OSMO_GEOOBJECT_C(v);
            }
            else if (!strcmp(a, "timestamp") || !strcmp(a, "tstamp") || !strcmp(a, "t")) {

                if (!memccpy(t, v, 0, T_MAXLEN))
                    throw std::length_error("timestamp too long");
            }
            else if (!strcmp(a, "version") || !strcmp(a, "v")) {
            
                this->v = OSMO_GEOOBJECT_V(v);
            }
			else if (!strcmp(a, "lon")) {
			
				lon = OSMO_GEOOBJECT_LON(v);
			}
			else if (!strcmp(a, "lat")) {
			
				lat = OSMO_GEOOBJECT_LAT(v);
			}
			else if (!GeoObjectRef::set_attr(a, v)) {

				return user->set_attr(a, v);
			}

			return true;
		}


		// SPECIAL METHODS
		void add_tag (Tag tag) {

			tags.push_back(tag);
		}

		private:
		struct tm tm;
		public:
		time_t get_tstamp_seconds () {

			if (t[0] == '\0') return -1;


			if (strptime(t, "%Y-%m-%dT%H:%M:%SZ", &tm) == NULL) {
				return -1;
			} else {
				return mktime(&tm);
			}
		}

		BSONObj getBson () {

			BSONObjBuilder b;
			GeoObjectRef::appendToBsonBuilder(&b);
			b
				.append("c", c)
				.append("t", (int)get_tstamp_seconds())
				.append("v", v)
				.append("u", user->getBson());

			if (!strcmp(p, "n")) {

				BSONObjBuilder geometryB;
				geometryB.append("type", "Point");
					BSONArrayBuilder geometryCoordsB;
					geometryCoordsB.append(lon);
					geometryCoordsB.append(lat);
				geometryB.append("coordinates", geometryCoordsB.arr());
				b.append("geometry", geometryB.obj());
			}

			if (strcmp(p, "n")) {
			
				b.append("parts", parts->getBson());
			}

			int len = (int)tags.size();

	        /*		BSONArrayBuilder indexB;
				for (int i = 0; i < len; i++) {

	                indexB.append(tags[i].getBson());
	            }
	        	b.append("tags", indexB.arr());*/
	        	BSONObjBuilder tagsB;
			for (int i = 0; i < len; i++) {

	        		tags[i].appendToBsonBuilder(&tagsB);
	        	}
	        	b.append("tags", tagsB.obj());

	        	

				///
			/*double y = yByLat(16, lat);
			if (y != NAN) {
				BSONArrayBuilder _tb;
				_tb
					.append(xByLon(16, lon))
					.append(lat);
				b.append("_t", _tb.arr());
			}*/
	        	
	        return b.obj();
		}
	};
}

#include "mongodb.h"