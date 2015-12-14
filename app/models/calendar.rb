class Calendar < ActiveRecord::Base
  belongs_to :user
  validates :uri, uniqueness: true

  def props
    propxml = self.propxml
    if propxml.length >= 2 
      ActiveSupport::JSON.decode(propxml)
    else
      {}
    end
  end

  def props=(v)
    self.propxml = ActiveSupport::JSON.encode(v)
  end
end
