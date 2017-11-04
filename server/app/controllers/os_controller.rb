class OsController < ApplicationController
  skip_before_action :authenticate

  def index
    @releases = MakeStack.settings[:os_releases]
  end
end
