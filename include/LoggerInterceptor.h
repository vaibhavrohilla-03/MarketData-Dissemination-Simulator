#pragma once

#include <grpcpp/grpcpp.h>
#include <grpcpp/support/interceptor.h>
#include <grpcpp/support/server_interceptor.h>


class LoggerInterceptor final : public grpc::experimental::Interceptor {

public:
	explicit LoggerInterceptor(grpc::experimental::ServerRpcInfo* info);

	void Intercept(grpc::experimental::InterceptorBatchMethods *methods) override;

};

class LoggerInterceptorFactory : public grpc::experimental::ServerInterceptorFactoryInterface {

public:

	grpc::experimental::Interceptor* CreateServerInterceptor(grpc::experimental::ServerRpcInfo* info) override;
};