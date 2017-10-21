class Integration < ApplicationRecord
  belongs_to :app

  SUPPORTED_SERVICES = %w(incoming_webhook outgoing_webhook ifttt slack datadog)
  INTEGRATION_TOKEN_LEN = 40
  INTEGRATION_TOKEN_PREFIX_LEN = 20

  validates :name, uniqueness: { scope: :app_id }
  validates :token_prefix, uniqueness: true
  validates :service, inclusion: { in: SUPPORTED_SERVICES }
  validate :token_prefix_is_prefix

  before_create :set_name

  def config_is_json
    JSON.parse(self.config)
  rescue
    errors.add(:config, "is not valid JSON.")
  end

  def token_prefix_is_prefix
    if self.token && !self.token.start_with?(self.token_prefix)
      errors.add(:token_prefix, "is not prefix of integration (BUG).")
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
