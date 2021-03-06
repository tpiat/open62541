/*
 * This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information.
 */

#include <signal.h>
#include <stdlib.h>

#ifdef UA_NO_AMALGAMATION
# include "ua_types.h"
# include "ua_server.h"
# include "logger_stdout.h"
# include "networklayer_tcp.h"
#else
# include "open62541.h"
#endif

UA_Boolean running = UA_TRUE;
UA_Logger logger;

static UA_StatusCode helloWorldMethod(const UA_NodeId objectId,
		const UA_Variant *input, UA_Variant *output) {
	UA_String *inputStr = (UA_String*) input->data;
	UA_String tmp = UA_STRING_ALLOC("Hello ");
	if (inputStr->length > 0) {
		tmp.data = realloc(tmp.data, tmp.length + inputStr->length);
		memcpy(&tmp.data[tmp.length], inputStr->data, inputStr->length);
		tmp.length += inputStr->length;
	}
	UA_Variant_setScalarCopy(output, &tmp, &UA_TYPES[UA_TYPES_STRING]);
	UA_String_deleteMembers(&tmp);
	UA_LOG_INFO(logger, UA_LOGCATEGORY_SERVER, "Hello World was called");
	return UA_STATUSCODE_GOOD;
}

static UA_StatusCode IncInt32ArrayValues(const UA_NodeId objectId,
		const UA_Variant *input, UA_Variant *output) {


	UA_Variant_setArrayCopy(output,input->data,5,&UA_TYPES[UA_TYPES_INT32]);
	for(int i = 0; i< input->arrayLength; i++){
		((UA_Int32*)output->data)[i] = ((UA_Int32*)input->data)[i] + 2;
	}

	return UA_STATUSCODE_GOOD;
}


static void stopHandler(int sign) {
    UA_LOG_INFO(logger, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = 0;
}
int main(int argc, char** argv) {
	signal(SIGINT, stopHandler); /* catches ctrl-c */

	/* initialize the server */
	UA_Server *server = UA_Server_new(UA_ServerConfig_standard);
	logger = Logger_Stdout_new();
	UA_Server_setLogger(server, logger);
	UA_Server_addNetworkLayer(server,
			ServerNetworkLayerTCP_new(UA_ConnectionConfig_standard, 16664));


	 /* add the method node with the callback */
	 UA_Argument inputArguments;
	 UA_Argument_init(&inputArguments);
	 inputArguments.arrayDimensionsSize = -1;
	 inputArguments.arrayDimensions = NULL;
	 inputArguments.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
	 inputArguments.description = UA_LOCALIZEDTEXT("en_US", "A String");
	 inputArguments.name = UA_STRING("MyInput");
	 inputArguments.valueRank = -1;

	 UA_Argument outputArguments;
	 UA_Argument_init(&outputArguments);
	 outputArguments.arrayDimensionsSize = -1;
	 outputArguments.arrayDimensions = NULL;
	 outputArguments.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
	 outputArguments.description = UA_LOCALIZEDTEXT("en_US", "A String");
	 outputArguments.name = UA_STRING("MyOutput");
	 outputArguments.valueRank = -1;
	 UA_NodeId createdNodeId;
	 UA_Server_addObjectNode(server,UA_QUALIFIEDNAME(1, "hello world object node"),UA_NODEID_NUMERIC(1,31415),
	 UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
	 UA_NODEID_NUMERIC(0,UA_NS0ID_OBJECTNODE),&createdNodeId);

	 UA_Server_addMethodNode(server, UA_QUALIFIEDNAME(1, "hello world"), UA_NODEID_NUMERIC(1,62541),
	 UA_EXPANDEDNODEID_NUMERIC(1, 31415), UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
	 &helloWorldMethod, 1, &inputArguments, 1, &outputArguments, NULL);

	 /* add another method node: output argument as 1d Int32 array*/
	// define input arguments
	UA_Argument_init(&inputArguments);
	inputArguments.arrayDimensionsSize = 1;
	UA_UInt32 * pInputDimensions = UA_UInt32_new();
	pInputDimensions[0] = 5;
	inputArguments.arrayDimensions = pInputDimensions;
	inputArguments.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
	inputArguments.description = UA_LOCALIZEDTEXT("en_US",
			"input an array with 5 elements, type int32");
	inputArguments.name = UA_STRING("int32 value");
	inputArguments.valueRank = 1;

	// define output arguments
	UA_Argument_init(&outputArguments);
	outputArguments.arrayDimensionsSize = 1;
	UA_UInt32 * pOutputDimensions = UA_UInt32_new();
	pOutputDimensions[0] = 5;
	outputArguments.arrayDimensions = pOutputDimensions;
	outputArguments.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
	outputArguments.description = UA_LOCALIZEDTEXT("en_US",
			"increment each array index");
	outputArguments.name = UA_STRING(
			"output is the array, each index is incremented");
	outputArguments.valueRank = 1;

	UA_Server_addMethodNode(server, UA_QUALIFIEDNAME(1, "IncInt32ArrayValues"),
			UA_NODEID_STRING(1, "IncInt32ArrayValues"),
			UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
			UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), &IncInt32ArrayValues,
			1, &inputArguments, 1, &outputArguments, NULL);

	/* start server */
	UA_StatusCode retval = UA_Server_run(server, 1, &running); //blocks until running=false

	/* ctrl-c received -> clean up */

        
    UA_UInt32_delete(pInputDimensions);
    UA_UInt32_delete(pOutputDimensions);
	UA_Server_delete(server);

	return retval;
}


