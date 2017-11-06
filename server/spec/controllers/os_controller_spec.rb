require 'rails_helper'

RSpec.describe OsController, type: :controller do
  describe '#index' do
    it 'returns OS releases' do
      get 'index'
      expect(JSON.parse(response.body)).to eq({ 'releases' => MakeStack.settings[:os_releases] })
    end
  end
end
