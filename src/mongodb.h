#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <expat.h>
#include "client/dbclient.h"

using namespace mongo;
using namespace osmo;




namespace osmo {

	namespace mongodb {

		typedef struct {
			DBClientConnection* c;
			const char* collection;
			GeoObject geoObject;
	    } context;

		const int BUFFER_SIZE = 10240 * 1;

		void init_object (GeoObject* geoObject, const XML_Char* element, const XML_Char** attrs) {

		    geoObject->reset();
		    geoObject->set_attr("p", element);
		    for (int i = 0; attrs[i]; i += 2) {

		    		geoObject->set_attr(attrs[i], attrs[i + 1]);
		    }
		}

		void XMLCALL startElement(void* data, const XML_Char* element, const XML_Char** attrs) {

			context* ct = (context*)data;
			GeoObject* geoObject = &ct->geoObject;

		    // order in the following "if" statements is based on frequency of tags in planet file
		    if (!strcmp(element, "nd")) {

		    		GeoObjectRef geoObjectRef;
		    		geoObjectRef.set_attr("p", "n");
		        for (int i = 0; attrs[i]; i += 2) {
		        
		        		geoObjectRef.set_attr(attrs[i], attrs[i + 1]);
		        }
		        geoObject->parts->add(geoObjectRef);
		    }
		    else if (!strcmp(element, "node")) {

		        init_object(geoObject, element, attrs);
		    }
		   	else if (!strcmp(element, "tag")) {

				Tag tag;
		        for (int i = 0; attrs[i]; i += 2) {
		        
		            tag.set_attr(attrs[i], attrs[i + 1]);
		        }
		        geoObject->add_tag(tag);
		    }
		    else if (!strcmp(element, "way")) {
		    
		        init_object(geoObject, element, attrs);
		    }
		    else if (!strcmp(element, "member")) {

				GeoObjectRef geoObjectRef;
				GeoObjectRel geoObjectRel;
		        for (int i = 0; attrs[i]; i += 2) {
		        
		        		geoObjectRef.set_attr(attrs[i], attrs[i + 1]);
		        		geoObjectRel.set_attr(attrs[i], attrs[i + 1]);
		        }
		        	geoObject->parts->add(geoObjectRef, geoObjectRel);
		    }
		    else if (!strcmp(element, "relation")) {
		    
				init_object(geoObject, element, attrs);
		    }
		}

		void XMLCALL endElement(void *data, const XML_Char* element) {

			context* ct = (context*)data;

		    if (!strcmp(element, "node") || !strcmp(element, "way") || !strcmp(element, "relation")) {

		        ct->c->insert(ct->collection, ct->geoObject.getBson());
		        //std::cout << ct->c->getLastErrorDetailed().jsonString() << " ";
		    }
		}

		void parse (int fd, DBClientConnection* c, const char* collection) {
		
		    bool done;
		    register context ct = {
		    		c,
		    		collection,
				GeoObject()
		    };

		    //c->dropCollection(collection);

		    XML_Parser parser = XML_ParserCreate(0);
		    if (!parser) {
		        throw std::runtime_error("Error creating parser");
		    }

		    XML_SetUserData(parser, (void*)&ct);

		    XML_SetElementHandler(parser, startElement, endElement);

		    do {
		        void *buffer = XML_GetBuffer(parser, BUFFER_SIZE);
		        if (buffer == 0) {
		            throw std::runtime_error("Out of memory");
		        }

		        int result = read(fd, buffer, BUFFER_SIZE);
		        if (result < 0) {
		            exit(1);
		        }
		        done = (result == 0);
		        if (XML_ParseBuffer(parser, result, done) == XML_STATUS_ERROR)
		        {
		            XML_Error errorCode = XML_GetErrorCode(parser);
		            long errorLine = XML_GetCurrentLineNumber(parser);
		            long errorCol = XML_GetCurrentColumnNumber(parser);
		            const XML_LChar *errorString = XML_ErrorString(errorCode);

		            std::stringstream errorDesc;
		            errorDesc << "XML parsing error at line " << errorLine << ":" << errorCol;
		            errorDesc << ": " << errorString;
		            throw std::runtime_error(errorDesc.str());
		        }
		    } while (!done);

		    XML_ParserFree(parser);
		    //error = "";
		}
		

		void osm2db (const char* file, const char* collection) {

			int fd = open(file, O_RDONLY);
			if (fd < 0) {

				std::cerr << "Can't open data file" << std::endl;
				exit(1);
			}

			DBClientConnection* c = new DBClientConnection;
			try {

				c->connect("localhost");
			} catch (DBException &e) {
		
				std::cerr << "Cannot connect: " << e.what() << std::endl;
				exit(1);
			}


			char *suffix = strrchr(file, '.');
			if (!strcmp(suffix, ".osm")) {

				parse(fd, c, collection);
			}
			/*else if (!strcmp(suffix, ".osc")) {

			}
			else if (!strcmp(suffix, ".pbf")) {

			}*/
			else {

				std::cerr << "Unknown file suffix: " << suffix << std::endl;
				exit(1);
			}


			close(fd);
		}
	}
}
