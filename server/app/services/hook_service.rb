module HookService
  extend self

  def invoke(integrations, hook, model, options)
    case model
    when Device
      case hook
      when :event_published
        handle_event_published(integrations, model, options)
      when :command_invoked
        handle_command_invoked(integrations, model, options)
      end
    end
  end

  private

  def handle_event_published(integrations, device, options)
    event = options[:event]
    body = options[:body]

    integrations.each do |integration|
      config = JSON.parse(integration.config)

      case integration.service
      when 'outgoing_webhook'
        CallOutgoingWebhookJob.perform_later(
          url: config['webhook_url'],
          body: { type: 'event_published', device: device.name, event: event, body: body },
          accept_configs: true,
          device: device
        )

      when 'ifttt'
        CallOutgoingWebhookJob.perform_later(
          url: "https://maker.ifttt.com/trigger/#{event}/with/key/#{config['key']}",
          body: { value1: body }
          )

      when 'slack'
        CallOutgoingWebhookJob.perform_later(
          url: config['webhook_url'],
          body: { text: "#{device.name} published event `#{event}`: `#{body}`" }
          )

      when 'datadog'
        CallOutgoingWebhookJob.perform_later(
          url: "https://app.datadoghq.com/api/v1/events",
          params: { api_key: config['api_key'] },
          body: { title: "#{device.name} published event `#{event}`", text: body },
        )
      end
    end
  end

  def handle_command_invoked(integrations, device, options)
    command_id = options[:command_id]
    return_value = options[:return_value]

    integrations.each do |integration|
      config = JSON.parse(integration.config)

      case integration.service
      when 'outgoing_webhook'
        CallOutgoingWebhookJob.perform_later(
          url: config['webhook_url'],
          body: { type: 'command_invoked', device: device.name, command_id: command_id, return_value: return_value },
          accept_configs: true,
          device: device
        )

      when 'ifttt'
        CallOutgoingWebhookJob.perform_later(
          url: "https://maker.ifttt.com/trigger/#{event}/with/key/#{config['key']}",
          body: { value1: return_value }
          )

      when 'slack'
        CallOutgoingWebhookJob.perform_later(
          url: config['webhook_url'],
          body: { text: "#{device.name} returned a command result: `#{return_value}` (`id=#{command_id}`)" }
          )

      when 'datadog'
        CallOutgoingWebhookJob.perform_later(
          url: "https://app.datadoghq.com/api/v1/events",
          params: { api_key: config['api_key'] },
          body: { title: "#{device.name} returned a command result `id=#{command_id}`", text: return_value },
        )
      end
    end
  end
end
