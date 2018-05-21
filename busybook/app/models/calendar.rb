class Calendar < ActiveRecord::Base
  belongs_to :user
  has_many :schedules, dependent: :destroy
  validates :uri, uniqueness: true

  def props
    props_json = self.props_json
    if props_json.length >= 2
      ActiveSupport::JSON.decode(props_json)
    else
      {}
    end
  end

  def props=(v)
    self.props_json = ActiveSupport::JSON.encode(v)
  end
end
