class CallOutgoingWebhookJob < ApplicationJob
  queue_as :default
  throttle limit: MakeStack.settings[:outgoing_webhook_limit_per_hour], period: 1.hour

  def execute(url:, params: {}, body:, accept_configs: false, device: nil)
    resp = RestClient.post(url, body.to_json, params: params, content_type: :json)

    # Update device configs.
    if accept_configs
      ActiveRecord::Base.transaction do
        configs = JSON.parse(resp.body).fetch("configs", {})
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

  rescue => e
    logger.warn "failed to call webhook: #{e}"
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
