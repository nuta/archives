import $ from 'jquery'

export function success(message) {
  $('success-notification').text(message);
  $('success-notification').slideDown(message, () => {
    setTimeout(() => {
      $('success-notification').slideUp()
    }, 5000)
  });
}
