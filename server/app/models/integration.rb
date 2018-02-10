class Integration < ApplicationRecord
  include Quota

  belongs_to :app

  SUPPORTED_SERVICES = %w(incoming_webhook outgoing_webhook ifttt thing_speak slack datadog sakuraio)
  INTEGRATION_TOKEN_LEN = 40
  INTEGRATION_TOKEN_PREFIX_LEN = 20
  INTEGRATIONS_MAX_NUM = 10

  quota scope: :app_id, limit: INTEGRATIONS_MAX_NUM

  validates :name,
    uniqueness: { scope: :app_id }

  validates :service,
    inclusion: { in: SUPPORTED_SERVICES }

  validate :validate_config_contents
    validates :config,
    length: { in: 0..1024 }

  validates :comment,
    length: { in: 0..256 },
    allow_nil: true

   validate :token_prefix_is_prefix
   validates :token_prefix,
     uniqueness: true,
     allow_nil: true

  after_initialize :set_token, if: :new_record?
  before_create :set_name

  def validate_config_contents
    begin
      config = JSON.parse(self.config)
    rescue
      errors.add(:config, "is not valid JSON.")
      return
    end

    case self.service
    when 'outgoing_webhook'
      unless URI::regexp(%w(http https)).match(config.dig('webhook_url'))
        errors.add(:config, "does not contain valid `webhook_url'")
      end
    when 'slack'
      unless URI::regexp(%w(http https)).match(config.dig('webhook_url'))
        errors.add(:config, "does not contain valid `webhook_url'")
      end
      unless (config.dig('webhook_url') || '').start_with('https://hooks.slack.com/services/')
        errors.add(:config, "`webhook_url' must starts with `https://hooks.slack.com/services/'")
      end
    when 'ifttt'
      unless config.dig('key')
        errors.add(:config, "does not contain `key''")
      end
    when 'thing_speak'
      unless config.dig('write_api_key')
        errors.add(:config, "does not contain `write_api_key''")
      end
    when 'datadog'
      unless config.dig('api_key')
        errors.add(:config, "does not contain `api_key''")
      end
    when 'sakuraio'
      unless config.dig('incoming_webhook_token')
        errors.add(:config, "does not contain `incoming_webhook_token''")
      end
      else
      errors.add(:config, "is not implemented in `validate_config_contents'.")
    end
  end

  def token_prefix_is_prefix
    if self.token && !self.token.start_with?(self.token_prefix)
      errors.add(:token_prefix, "is not prefix of integration (BUG).")
    end
  end

  def set_token
    self.reset_token
  end

  def set_name
    app_integrations = Integration.where(app: self.app, service: self.service)
    minor_id = (app_integrations.count || 0) + 1
    self.name = self.service + ((minor_id == 1) ? '' : minor_id.to_s)
  end

  def reset_token
    token = RandomIdGenerator::generate(INTEGRATION_TOKEN_LEN)
    self.token_prefix = token[0, INTEGRATION_TOKEN_PREFIX_LEN]
    self.token = token
  end

  def self.lookup_by_token(token)
    prefix = token[0, INTEGRATION_TOKEN_PREFIX_LEN]
    integration = Integration.find_by_token_prefix(prefix)

    if integration && ActiveSupport::SecurityUtils.secure_compare(integration.token, token)
      integration
    else
      nil
    end
  end
end
