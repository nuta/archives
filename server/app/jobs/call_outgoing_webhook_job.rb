class CallOutgoingWebhookJob < ApplicationJob
  queue_as :outgoing_webhook
  throttle limit: MakeStack.settings[:outgoing_webhook_limit_per_hour], period: 1.hour

  rescue_from(StandardError) do |e|
    backtrace = e.backtrace.join("\n")
    Rails.logger.error "Failed to invoke a webhook: #{e.to_s}\n#{backtrace}"
  end

  def execute(url:, params: {}, body: nil, accept_configs: false, device: nil)
    if body
      resp = RestClient.post(url, body.to_json, params: params, content_type: :json)
    else
      resp = RestClient.post(url, params: params)
    end

    # Update device configs.
    if accept_configs
      ActiveRecord::Base.transaction do
        begin
          configs = JSON.parse(resp.body).fetch("configs", {})
        rescue
          # The response body is not JSON.
          return
        end

        unless configs.is_a?(Hash)
          raise "`configs' received from a webhook is not Hash"
        end

        configs.each do |key, value|
          data_type = determine_data_type(value)
          config = device.configs.where(key: key).first_or_initialize
          config.value = value.to_s
          config.data_type = data_type
          config.save!
        end
      end
    end
  end

  private

  def determine_data_type(value)
    case value
    when String then 'string'
    when TrueClass, FalseClass then 'bool'
    when Float then 'float'
    when Integer then 'integer'
    else raise 'unknown data type'
    end
  end
end
