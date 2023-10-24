void SidecarApi::RequestLogHandler(const httplib::Request &req, const httplib::Response &res)
{
	if (!req.path.empty()) {
		std::vector<std::string> params;
		for (auto                &p: req.params) {
			params.emplace_back(fmt::format("{}={}", p.first, p.second));
		}

		LogInfo(
			"[API] Request [{}] [{}{}] via [{}:{}]",
			res.status,
			req.path,
			(!params.empty() ? "?" + Strings::Join(params, "&") : ""),
			req.remote_addr,
			req.remote_port
		);
	}
}
