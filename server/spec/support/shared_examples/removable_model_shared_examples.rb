require 'rails_helper'

RSpec.shared_examples 'a removable model' do |models_to_be_removed|
  let!(:other_user) { create(:heavy_user) }

  models_to_be_removed.each do |model|
    it "destroys #{model.name}" do
      count = subject.try(model.name.underscore.pluralize).count
      expect {
        subject.destroy
      }.to change(model, :count).by(count)
    end
  end
end
