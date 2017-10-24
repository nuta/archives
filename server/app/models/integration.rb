class Integration < ApplicationRecord
  belongs_to :app

  SUPPORTED_SERVICES = %w(incoming_webhook outgoing_webhook ifttt slack datadog)
  INTEGRATION_TOKEN_LEN = 40
  INTEGRATION_TOKEN_PREFIX_LEN = 20
  INTEGRATIONS_MAX_NUM = 10

  validates :name, uniqueness: { scope: :app_id }
  validates :token_prefix, uniqueness: true, allow_nil: true
  validates :service, inclusion: { in: SUPPORTED_SERVICES }
  validate :token_prefix_is_prefix
  validate :validate_config_contents
  validate :validate_num_of_integrations, on: :create

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
    when 'ifttt'
      unless config.dig('key')
        errors.add(:config, "does not contain `key''")
      end
    when 'datadog'
      unless config.dig('api_key')
        errors.add(:config, "does not contain `api_key''")
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

  def validate_num_of_integrations
    if self.app && self.app.integrations.count >= INTEGRATIONS_MAX_NUM
      errors.add(:integrations, "are too many.")
    end
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
