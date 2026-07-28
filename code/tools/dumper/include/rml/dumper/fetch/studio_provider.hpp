#pragma once

#include "rml/dumper/fetch/deploy_client.hpp"

namespace rml::dumper::fetch
{
	struct ResolvedStudio
	{
		std::filesystem::path executable;
		StudioVersion version;
	};

	class StudioProvider
	{
	public:
		virtual ~StudioProvider() = default;

		[[nodiscard]] virtual std::expected<ResolvedStudio, Error> resolve() = 0;
	};

	class LocalStudioProvider final : public StudioProvider
	{
	public:
		explicit LocalStudioProvider(std::filesystem::path path) :
		    m_path(std::move(path))
		{
		}

		[[nodiscard]] std::expected<ResolvedStudio, Error> resolve() override;

	private:
		std::filesystem::path m_path;
	};

	class RemoteStudioProvider final : public StudioProvider
	{
	public:
		RemoteStudioProvider(const HttpClient& client, DeployClient deploy, Deployment deployment,
		                     std::filesystem::path cache) :
		    m_client(&client),
		    m_deploy(std::move(deploy)),
		    m_deployment(deployment),
		    m_cache(std::move(cache))
		{
		}

		[[nodiscard]] std::expected<ResolvedStudio, Error> resolve() override;

	private:
		const HttpClient* m_client;
		DeployClient m_deploy;
		Deployment m_deployment;
		std::filesystem::path m_cache;
	};
}
