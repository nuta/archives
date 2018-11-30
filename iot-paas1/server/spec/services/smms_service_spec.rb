require 'rails_helper'

RSpec.describe SMMSService do
  describe "generate_variable_length" do
    it "generates valid length" do
      length = SMMSService.send(:generate_variable_length, "a")
      expect(length).to eq("\x01")

      length = SMMSService.send(:generate_variable_length, "a" * 127)
      expect(length).to eq("\x7f")

      length = SMMSService.send(:generate_variable_length, "a" * 128)
      expect(length).to eq([0x80, 0x01].pack('C*'))

      length = SMMSService.send(:generate_variable_length, "a" * 129)
      expect(length).to eq([0x81, 0x01].pack('C*'))
    end
  end

  describe "parses_variable_length" do
    it "parses valid length correctly" do
      length, length_length = SMMSService.send(:parse_variable_length, [0x08].pack('C*'))
      expect(length).to eq(8)
      expect(length_length).to eq(1)

      length, length_length = SMMSService.send(:parse_variable_length, [0x80, 0x01].pack('C*'))
      expect(length).to eq(128)
      expect(length_length).to eq(2)

      buf = SMMSService.send(:generate_variable_length, "a" * 127)
      length, length_length = SMMSService.send(:parse_variable_length, buf)
      expect(length).to eq(127)
      expect(length_length).to eq(1)

      buf = SMMSService.send(:generate_variable_length, "a" * 1600)
      length, length_length = SMMSService.send(:parse_variable_length, buf)
      expect(length).to eq(1600)
      expect(length_length).to eq(2)

      buf = SMMSService.send(:generate_variable_length, "a" * 50000)
      length, length_length = SMMSService.send(:parse_variable_length, buf)
      expect(length).to eq(50000)
      expect(length_length).to eq(3)
    end
  end
end
