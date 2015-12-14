class Calendar < ActiveRecord::Base
  belongs_to :user
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

  def Calendar.find_by_name(name)
    Calendar.all.find_each do |cal|
      if name == cal.props['displayname']
        return cal
      end
    end

    nil
  end
end
