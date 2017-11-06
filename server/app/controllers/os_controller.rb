class OsController < ApplicationController
  skip_before_action :authenticate

  def index
    @releases = MakeStack.os_releases
  end
end
