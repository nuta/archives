module RandomIdGenerator
  def self.generate(n)
    urlsafe_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_~"

    str = ""
    while str.length < n
      str << urlsafe_chars[SecureRandom.random_number(urlsafe_chars.length)]
    end

    str
  end
end