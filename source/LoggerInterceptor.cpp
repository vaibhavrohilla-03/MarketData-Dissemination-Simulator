#include "LoggerInterceptor.h"

#include <string>

LoggerInterceptor::LoggerInterceptor(grpc::experimental::ServerRpcInfo* info) {
		std::string method = info->method();

		if(method == "unknown") {

			std::cout << "Unimplemented RPC called" << "\n";
			return;
		}

		std::cout << "RPC called : " << method << "\n"; 
}

void LoggerInterceptor::Intercept(grpc::experimental::InterceptorBatchMethods *methods) {

	methods->Proceed();
}


grpc::experimental::Interceptor* LoggerInterceptorFactory::CreateServerInterceptor(grpc::experimental::ServerRpcInfo* info) {

	return new LoggerInterceptor(info);

}