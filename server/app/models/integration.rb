class Integration < ApplicationRecord
    belongs_to :app

    SUPPORTED_SERVICES = %w(outgoing_webhook ifttt slack)

    validates :service, inclusion: { in: SUPPORTED_SERVICES },
                        uniqueness: { scope: :app_id }
    validate :config_is_json
    
    # TODO: validate contents in the config JSON

    def config_is_json
      JSON.parse(self.config)
    rescue
      errors.add(:config, "is not valid JSON.")
    end
end
